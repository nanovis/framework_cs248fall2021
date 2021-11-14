#include "webgpu.h"
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
// You can use glm or using your implementation in previous assignments for calculation
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#define GLg_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>
using namespace std;

//----------- WEBGPU variables ---------------------------------------

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapchain;

WGPURenderPipeline pipeline;
WGPUBuffer vertBuf; // vertex buffer with triangle position and colours
WGPUBuffer indxBuf; // index buffer
WGPUBuffer uRotBuf; // rotation buffer
WGPUBindGroup* bindGroup; 


/**
 * Current rotation angle (in degrees, updated per frame).
 */

float rotDeg = 0.0f;

const int viewWidth = 1024, viewHeight = 768;

std::string simple_vert_wgsl;
std::string simple_frag_wgsl;

vector <glm::vec3> points;
vector <glm::vec3> colors;

// This is just an example vertex data
void setVertexData()
{
	points.emplace_back(glm::vec3(-0.5f, -0.5f, 0.0f));
	points.emplace_back(glm::vec3( 0.5f, -0.5f, 0.0f));
	points.emplace_back(glm::vec3(-0.5f,  0.5f, 0.0f));
	points.emplace_back(glm::vec3( 0.5f,  0.5f, 0.0f));
	colors.emplace_back(glm::vec3(0.583f, 0.771f, 0.014f));
	colors.emplace_back(glm::vec3(0.609f, 0.115f, 0.436f));
	colors.emplace_back(glm::vec3(0.327f, 0.483f, 0.844f));
	colors.emplace_back(glm::vec3(0.822f, 0.569f, 0.201f));
}

static void setupShader()
{
	

	// Simple shaders
	simple_vert_wgsl = R"(
	
		[[block]]
		struct VertexIn {
			[[location(0)]] aPos : vec3<f32>;
			[[location(1)]] aColor : vec3<f32>;
		};
		struct VertexOut {
			[[builtin(position)]] Position : vec4<f32>;
			[[location(0)]] vColor : vec3<f32>;
		};
		
		[[block]]
		struct Rotation {
			degs : f32;
		};
		[[group(0), binding(0)]] var<uniform> uRot : Rotation;
		
		let PI : f32 = 3.141592653589793;
		fn radians(degs : f32) -> f32 {
			return (degs * PI) / 180.0;
		}

		
		[[stage(vertex)]]
		fn main(input : VertexIn) -> VertexOut {
			var rads : f32 = radians(uRot.degs);
			var cosA : f32 = cos(rads);
			var sinA : f32 = sin(rads);
			var rot : mat3x3<f32> = mat3x3<f32>(
				vec3<f32>( 1.0, 0.0, 0.0),
				vec3<f32>( 0.0, cosA, -sinA),
				vec3<f32>( 0.0, sinA, cosA));
			var output : VertexOut;
			output.Position = vec4<f32>(rot * input.aPos, 1.0);
			output.vColor = input.aColor;
			return output;
		}
	)";

	simple_frag_wgsl = R"(
		[[stage(fragment)]]
		fn main([[location(0)]] vColor : vec3<f32>) -> [[location(0)]] vec4<f32> {
			return vec4<f32>(vColor, 1.0);
		}
	)";

}


/**
 * Helper to create a shader from WGSL source.
 *
 * \param[in] code WGSL shader source
 * \param[in] label optional shader name
 */
static WGPUShaderModule createShader(const char* const code, const char* label = nullptr) {
	WGPUShaderModuleWGSLDescriptor wgsl = {};
	wgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	wgsl.source = code;
	WGPUShaderModuleDescriptor desc = {};
	desc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgsl);
	desc.label = label;
	return wgpuDeviceCreateShaderModule(device, &desc);
}

/**
 * Helper to create a buffer.
 *
 * \param[in] data pointer to the start of the raw data
 * \param[in] size number of bytes in \a data
 * \param[in] usage type of buffer
 */
static WGPUBuffer createBuffer(const void* data, size_t size, WGPUBufferUsage usage) {
	WGPUBufferDescriptor desc = {};
	desc.usage = WGPUBufferUsage_CopyDst | usage;
	desc.size = size;
	WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);
	wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
	return buffer;
}


/**
 * Helper to create a texture.
 *
 * \param[in] 
 * \param[in] 
 * \param[in] 
 */

static WGPUTexture createTexture(unsigned char* data, unsigned int w, unsigned int h) {
	return nullptr;
}


static void createDepthTexture() {
	return;
}

/**
 * Bare minimum pipeline to draw a triangle using the above shaders.
 */
static void createPipelineAndBuffers() {
	// compile shaders
	// NOTE: these are now the WGSL shaders (tested with Dawn and Chrome Canary)

	setupShader();

	WGPUShaderModule vertMod = createShader(simple_vert_wgsl.c_str());
	WGPUShaderModule fragMod = createShader(simple_frag_wgsl.c_str());


	// bind group layout for uniform
	WGPUBufferBindingLayout buf = {};
	buf.type = WGPUBufferBindingType_Uniform;

	WGPUBindGroupLayoutEntry bglEntry = {};
	bglEntry.binding = 0;
	bglEntry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bglEntry.buffer = buf;
	bglEntry.sampler = { 0 };

	WGPUBindGroupLayoutDescriptor bglDesc = {};
	bglDesc.entryCount = 1;
	bglDesc.entries = &bglEntry;
	WGPUBindGroupLayout bindGroupLayoutUniform = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

	// add all uniform layouts
	int numUniformLayouts = 1;
	WGPUBindGroupLayout* allLayouts = new WGPUBindGroupLayout[numUniformLayouts];
	for (int i = 0; i < numUniformLayouts; i++)
		allLayouts[i] = bindGroupLayoutUniform;
	
	// pipeline layout (used by the render pipeline, released after its creation): remember to add all uniform layout to pipeline layout
	WGPUPipelineLayoutDescriptor layoutDesc = {};
	layoutDesc.bindGroupLayoutCount = numUniformLayouts;
	layoutDesc.bindGroupLayouts = allLayouts;
	WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);

	// describe vertex buffer layouts: Need to care about the memory layout
	WGPUVertexAttribute vertAttrs[2] = {};
	vertAttrs[0].format = WGPUVertexFormat_Float32x3;
	vertAttrs[0].offset = 0;
	vertAttrs[0].shaderLocation = 0;
	vertAttrs[1].format = WGPUVertexFormat_Float32x3;
	vertAttrs[1].offset = 4 * sizeof(float);
	vertAttrs[1].shaderLocation = 1;

	WGPUVertexBufferLayout vertexBufferLayout = {};
	vertexBufferLayout.arrayStride = 8 * sizeof(float);
	vertexBufferLayout.attributeCount = 2;
	vertexBufferLayout.attributes = vertAttrs;

	// create the vertex data (position - vec3, color - vec3)
	
	setVertexData();
	float* vertData = new float[points.size() * 8];
	int index = 0;
	int i;

	// Memory layout
	for (i = 0; i < points.size(); i++)
	{
		vertData[index] = float(points[i].x);
		vertData[index + 1] = float(points[i].y);
		vertData[index + 2] = float(points[i].z);
		vertData[index + 3] = 0.0f;
		vertData[index + 4] = float(colors[i].x);
		vertData[index + 5] = float(colors[i].y);
		vertData[index + 6] = float(colors[i].z);
		vertData[index + 7] = 1.0f;
		index += 8;
	}
	
	// create the indices data: the index to draw each triangle
	uint16_t const indxData[] = {
		0, 1, 2,
		1, 3, 2
	};


	vertBuf = createBuffer(vertData, points.size() * 8 * sizeof(float), WGPUBufferUsage_Vertex);
	indxBuf = createBuffer(indxData, 6 * sizeof(uint16_t), WGPUBufferUsage_Index);

	// create the uniform bind group (note 'rotDeg' is copied here, not bound in any way)
	uRotBuf = createBuffer(&rotDeg, sizeof(rotDeg), WGPUBufferUsage_Uniform);

	WGPUBindGroupEntry bgRotationEntry = {};
	bgRotationEntry.binding = 0;
	bgRotationEntry.buffer = uRotBuf;
	bgRotationEntry.offset = 0;
	bgRotationEntry.size = sizeof(rotDeg);

	WGPUBindGroupDescriptor bgRotationDesc = {};
	bgRotationDesc.layout = bindGroupLayoutUniform;
	bgRotationDesc.entryCount = 1;
	bgRotationDesc.entries = &bgRotationEntry;


	bindGroup = new WGPUBindGroup[numUniformLayouts];
	bindGroup[0] = wgpuDeviceCreateBindGroup(device, &bgRotationDesc);


	// last bit of clean-up
	wgpuBindGroupLayoutRelease(bindGroupLayoutUniform);


	// Vertex state
	WGPUVertexState vertex = {};
	vertex.module = vertMod;
	vertex.entryPoint = "main";
	vertex.bufferCount = 1;
	vertex.buffers = &vertexBufferLayout;


	// Fragment state
	WGPUBlendState blend = {};
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = WGPUBlendFactor_One;
	blend.color.dstFactor = WGPUBlendFactor_Zero;
	blend.alpha.operation = WGPUBlendOperation_Add;
	blend.alpha.srcFactor = WGPUBlendFactor_One;
	blend.alpha.dstFactor = WGPUBlendFactor_Zero;


	WGPUColorTargetState colorTarget = {};
	colorTarget.format = webgpu::getSwapChainFormat(device);
	colorTarget.blend = &blend;
	colorTarget.writeMask = WGPUColorWriteMask_All;

	WGPUFragmentState fragment = {};
	fragment.module = fragMod;
	fragment.entryPoint = "main";
	fragment.targetCount = 1;
	fragment.targets = &colorTarget;


#ifdef __EMSCRIPTEN__
	WGPURenderPipelineDescriptor desc = {};
#else
	WGPURenderPipelineDescriptor desc = {};
#endif
	desc.vertex = vertex;
	desc.fragment = &fragment;
	desc.layout = pipelineLayout;
	// Other states

	// Primitive state
	desc.primitive.frontFace = WGPUFrontFace_CCW;
	desc.primitive.cullMode = WGPUCullMode_None;
	desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

	// Depth Stencil state: You can add depth test in here


	// Multisample state
	desc.multisample.count = 1;
	desc.multisample.mask = 0xFFFFFFFF;
	desc.multisample.alphaToCoverageEnabled = false;

	
#ifdef __EMSCRIPTEN__
	pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);
#else
	pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);
#endif

	// partial clean-up 
	wgpuPipelineLayoutRelease(pipelineLayout);

	wgpuShaderModuleRelease(fragMod);
	wgpuShaderModuleRelease(vertMod);
}

/**
 * Draws using the above pipeline and buffers.
 */
static bool redraw() {
	WGPUTextureView backBufView = wgpuSwapChainGetCurrentTextureView(swapchain);			// create textureView

	WGPURenderPassColorAttachment colorDesc = {};
	colorDesc.view = backBufView;
	colorDesc.loadOp = WGPULoadOp_Clear;
	colorDesc.storeOp = WGPUStoreOp_Store;
	colorDesc.clearColor.r = 0.0f;
	colorDesc.clearColor.g = 0.0f;
	colorDesc.clearColor.b = 0.0f;
	colorDesc.clearColor.a = 1.0f;

	// You can add depth texture in here

	WGPURenderPassDescriptor renderPass = {};
	renderPass.colorAttachmentCount = 1;
	renderPass.colorAttachments = &colorDesc;


	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);			// create encoder
	WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPass);	// create pass


	// update the rotation
	rotDeg += 0.1f;
	if (rotDeg >= 360.f)
		rotDeg = 0.0f;
	wgpuQueueWriteBuffer(queue, uRotBuf, 0, &rotDeg, sizeof(rotDeg));

	// draw the object (comment these five lines to simply clear the screen)
	wgpuRenderPassEncoderSetPipeline(pass, pipeline);
	wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup[0], 0, 0);
	wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertBuf, 0, 0);
	wgpuRenderPassEncoderSetIndexBuffer(pass, indxBuf, WGPUIndexFormat_Uint16, 0, 0);
	// Instancing checking
	wgpuRenderPassEncoderDrawIndexed(pass, 6, 1, 0, 0, 0);

	wgpuRenderPassEncoderEndPass(pass);
	wgpuRenderPassEncoderRelease(pass);														// release pass
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);				// create commands
	wgpuCommandEncoderRelease(encoder);														// release encoder

	wgpuQueueSubmit(queue, 1, &commands);
	wgpuCommandBufferRelease(commands);														// release commands


#ifndef __EMSCRIPTEN__
	wgpuSwapChainPresent(swapchain);
#endif
	wgpuTextureViewRelease(backBufView);													// release textureView

	return true;
}



extern "C" int __main__(int /*argc*/, char* /*argv*/[]) {

	//----------- Draw windows and update scene ------------
	if (window::Handle wHnd = window::create(viewWidth, viewHeight, "Hello CS248")) {
		if ((device = webgpu::create(wHnd))) {

			queue = wgpuDeviceGetQueue(device);
			swapchain = webgpu::createSwapChain(device);

			createPipelineAndBuffers();


			window::show(wHnd);
			window::loop(wHnd, redraw);


#ifndef __EMSCRIPTEN__
			wgpuBindGroupRelease(bindGroup[0]);
			wgpuBufferRelease(uRotBuf);
			wgpuBufferRelease(indxBuf);
			wgpuBufferRelease(vertBuf);
			wgpuRenderPipelineRelease(pipeline);
			wgpuSwapChainRelease(swapchain);
			wgpuQueueRelease(queue);
			wgpuDeviceRelease(device);
#endif
		}
#ifndef __EMSCRIPTEN__
		window::destroy(wHnd);
#endif
	}


	return 0;
}
