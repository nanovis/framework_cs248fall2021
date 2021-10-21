#include "webgpu.h"
#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>
using namespace std;

#include "rasterization/Vec3.h"
#include "rasterization/Matrix4.h"
#include "rasterization/Light.h"
#include "rasterization/Color.h"
#include "rasterization/Camera.h"
#include "rasterization/Mesh.h"
#include <thread>         
#include <chrono>  


// Set up view plane
const int viewWidth = 1024, viewHeight = 768;
const double viewLeft = -2.0f, viewRight = 2.0f, viewBottom = -1.5f, viewTop = 1.5f;
const double aspect = (double)viewWidth / (double)viewHeight;
const double FoV = 45.f;
const double nearPlane = -1.0f, farPlane = -1000.0f;
const double INF = std::numeric_limits<double>::infinity();

//----------- Objects ----------------------------------------------
// #NOTE: You can find other material in this link: http://www.barradeau.com/nicoptere/dump/materials.html
Material emerald(Color(0.0215f, 0.1745f, 0.0215f), Color(0.07568f, 0.61424f, 0.07568f),
	Color(0.633f, 0.727811f, 0.633f), 76.8f);
// #NOTE: Depth buffer
double* depthBuffer = new double[viewWidth * viewHeight];
// #NOTE: Normal buffer
vector<Vec3> normalBuffer;

// #NOTE: Color buffer
vector<Color> colorBuffer;

// #NOTE: You can draw 3 objects in one scene or one object in one scene
// Hint: Testing with sphere first
Mesh sphere;
Mesh beacon;
Mesh teapot;

// #TODO: Change these matrices for each object so that we can see these objects in camera space
Matrix4 sphereModel;
Matrix4 normalSphere;

Matrix4 beaconModel;
Matrix4 normalBeacon;

Matrix4 teapotModel;
Matrix4 normalTeapot;

//----------- Camera -----------------------------------------------
Vec3 eye_postion(4.0f, 3.0f, 3.0f);
Vec3 gaze_direction(-4.0f, -3.0f, -3.0f);
Vec3 view_up(0.0f, 1.0f, 0.0f);
Camera camera(eye_postion, gaze_direction, view_up);

// Arcball Matrix: will be composited with View matrix from Camera 
Matrix4 Arcball;
Matrix4 View = camera.calculateViewMatrix();


// #NOTE: Testing with two projective methods, should start with orthographic first
// You can define additional variables in here for checking which projective method is used.

// #TODO: Uncomment the line below if using orthorgraphic projection
// Matrix4 Projection = camera.calculateOrthographicMatrix(viewLeft, viewRight, viewBottom, viewTop, nearPlane, farPlane);

// #TODO: Uncomment the line below if using perspective projection
//Matrix4 Projection = camera.calculatePerspectiveMatrix(FoV, aspect, nearPlane, farPlane);

// #NOTE: M_vp
Matrix4 Viewport;

//----------- Light ------------------------------------------------
Light light(Vec3(5.0f, 5.0f, 0.0f), Color(1.0f));
Matrix4 lightModel;

//----------- Result image ------------------------------------------
unsigned char* img = new unsigned char[viewWidth * viewHeight * 4];

//----------- Function for handling user interaction ----------------
void mouseClickHandler(int, int, int, int);
void keyPressHandler(int, int);

//----------- Rasterization ------------------------------------------

// #TODO: Compute Viewport matrix for Viewport transformation M_vp
void calculateViewPortMatrix(int viewWidth, int viewHeight, Matrix4& Viewport)
{

}

// #TODO: Define function to compute Arcball matrix for camera

// #TODO: Compute color
// #NOTE: Ambient light intensity is the same with light intensity
Color shade(Vec3 pos, Vec3 normal, Camera cam, Light light, Material m) {
	Color c;
	return c;
}

// #TODO: Implement rasterization
void rasterization(Mesh object, Matrix4 objectModel, Matrix4 normalMatrix) {
}

//----------- WEBGPU variables ---------------------------------------

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapchain;

WGPURenderPipeline pipeline;
WGPUBuffer vertBuf; // vertex buffer with position
WGPUBuffer indxBuf; // index buffer
WGPUBindGroup bindGroup;

WGPUTexture tex; // Texture
WGPUSampler samplerTex;
WGPUTextureView texView;
WGPUExtent3D texSize = {};
WGPUTextureDescriptor texDesc = {};
WGPUTextureDataLayout texDataLayout = {};
WGPUImageCopyTexture texCopy = {};

/*
* Shaders
*/
static char const rectangle_vert_wgsl[] = R"(
	[[block]]
	struct VertexIn {
		[[location(0)]] aPos : vec2<f32>;
		[[location(1)]] aTex : vec2<f32>;
	};
	struct VertexOut {
		[[location(0)]] vTex : vec2<f32>;
		[[builtin(position)]] Position : vec4<f32>;
	};
	[[stage(vertex)]]
	fn main(input : VertexIn) -> VertexOut {
		var output : VertexOut;
		output.Position = vec4<f32>(vec3<f32>(input.aPos, 1.0), 1.0);
		output.vTex = input.aTex;
		return output;
	}
)";

static char const rectangle_frag_wgsl[] = R"(
	[[group(0), binding(0)]]
	var tex: texture_2d<f32>;
	[[group(0), binding(1)]]
	var sam: sampler;

	[[stage(fragment)]]
	fn main([[location(0)]] vTex : vec2<f32>) -> [[location(0)]] vec4<f32> {
		return textureSample(tex, sam, vTex);
	}
)";

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
	desc.size  = size;
	WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);
	wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
	return buffer;
}

static WGPUTexture createTexture(unsigned char* data, unsigned int w, unsigned int h) {
	texSize.depthOrArrayLayers = 1;
	texSize.height = h;
	texSize.width = w;

	texDesc.sampleCount = 1;
	texDesc.mipLevelCount = 1;
	texDesc.dimension = WGPUTextureDimension_2D;
	texDesc.size = texSize;
	texDesc.usage = WGPUTextureUsage_Sampled | WGPUTextureUsage_CopyDst;
	texDesc.format = WGPUTextureFormat_RGBA8Unorm;

	texDataLayout.offset = 0;
	texDataLayout.bytesPerRow = 4 * w;
	texDataLayout.rowsPerImage = h;

	texCopy.texture = wgpuDeviceCreateTexture(device, &texDesc);

	wgpuQueueWriteTexture(queue, &texCopy, data, w * h * 4, &texDataLayout, &texSize);
	return texCopy.texture;
}

/**
 * Bare minimum pipeline to draw a triangle using the above shaders.
 */
static void createPipelineAndBuffers(unsigned char* data, unsigned int w, unsigned int h) {
	// compile shaders
	// NOTE: these are now the WGSL shaders (tested with Dawn and Chrome Canary)
	WGPUShaderModule vertMod = createShader(rectangle_vert_wgsl);
	WGPUShaderModule fragMod = createShader(rectangle_frag_wgsl);

	//===================================================================

	tex = createTexture(data, w, h);

	WGPUTextureViewDescriptor texViewDesc = {};
	texViewDesc.dimension = WGPUTextureViewDimension_2D;
	texViewDesc.format = WGPUTextureFormat_RGBA8Unorm;

	texView = wgpuTextureCreateView(tex, &texViewDesc);


	WGPUSamplerDescriptor samplerDesc = {};
	samplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
	samplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
	samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
	samplerDesc.magFilter = WGPUFilterMode_Linear;
	samplerDesc.minFilter = WGPUFilterMode_Nearest;
	samplerDesc.mipmapFilter = WGPUFilterMode_Nearest;
	samplerDesc.lodMaxClamp = 32;
	samplerDesc.lodMinClamp = 0;
	samplerDesc.compare = WGPUCompareFunction_Undefined;
	samplerDesc.maxAnisotropy = 1;
	
	samplerTex = wgpuDeviceCreateSampler(device, &samplerDesc);

	WGPUSamplerBindingLayout samplerLayout = {};
	samplerLayout.type = WGPUSamplerBindingType_Filtering;

	WGPUTextureBindingLayout texLayout = {};
	texLayout.sampleType = WGPUTextureSampleType_Float;
	texLayout.viewDimension = WGPUTextureViewDimension_2D;
	texLayout.multisampled = false;

	WGPUBindGroupLayoutEntry bglTexEntry = {};
	bglTexEntry.binding = 0;
	bglTexEntry.visibility = WGPUShaderStage_Fragment;
	bglTexEntry.texture = texLayout;

	WGPUBindGroupLayoutEntry bglSamplerEntry = {};
	bglSamplerEntry.binding = 1;
	bglSamplerEntry.visibility = WGPUShaderStage_Fragment;
	bglSamplerEntry.sampler = samplerLayout;

	WGPUBindGroupLayoutEntry* allBgLayoutEntries = new WGPUBindGroupLayoutEntry[2];
	allBgLayoutEntries[0] = bglTexEntry;
	allBgLayoutEntries[1] = bglSamplerEntry;

	//=======================================================================

	WGPUBindGroupLayoutDescriptor bglDesc = {};
	bglDesc.entryCount = 2;
	bglDesc.entries = allBgLayoutEntries;
	WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

	// pipeline layout (used by the render pipeline, released after its creation)
	WGPUPipelineLayoutDescriptor layoutDesc = {};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = &bindGroupLayout;
	WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);

	// describe buffer layouts
	WGPUVertexAttribute vertAttrs[2] = {};
	vertAttrs[0].format = WGPUVertexFormat_Float32x2;
	vertAttrs[0].offset = 0;
	vertAttrs[0].shaderLocation = 0;
	vertAttrs[1].format = WGPUVertexFormat_Float32x2;
	vertAttrs[1].offset = 2 * sizeof(float);
	vertAttrs[1].shaderLocation = 1;
	WGPUVertexBufferLayout vertexBufferLayout = {};
	vertexBufferLayout.arrayStride = 4 * sizeof(float);
	vertexBufferLayout.attributeCount = 2;
	vertexBufferLayout.attributes = vertAttrs;

	// Fragment state
	WGPUBlendState blend = {};
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = WGPUBlendFactor_One;
	blend.color.dstFactor = WGPUBlendFactor_One;
	blend.alpha.operation = WGPUBlendOperation_Add;
	blend.alpha.srcFactor = WGPUBlendFactor_One;
	blend.alpha.dstFactor = WGPUBlendFactor_One;

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
	desc.fragment = &fragment;

	// Other state
	desc.layout = pipelineLayout;
	desc.depthStencil = nullptr;

	desc.vertex.module = vertMod;
	desc.vertex.entryPoint = "main";
	desc.vertex.bufferCount = 1;
	desc.vertex.buffers = &vertexBufferLayout;

	desc.multisample.count = 1;
	desc.multisample.mask = 0xFFFFFFFF;
	desc.multisample.alphaToCoverageEnabled = false;

	desc.primitive.frontFace = WGPUFrontFace_CCW;
	desc.primitive.cullMode = WGPUCullMode_None;
	desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

#ifdef __EMSCRIPTEN__
	pipeline = wgpuDeviceCreateRenderPipeline (device, &desc);
#else
	pipeline = wgpuDeviceCreateRenderPipeline (device, &desc);
#endif

	// partial clean-up (just move to the end, no?)
	wgpuPipelineLayoutRelease(pipelineLayout);

	wgpuShaderModuleRelease(fragMod);
	wgpuShaderModuleRelease(vertMod);
	
	// create the buffers (position[2], tex_coords[2])
	float const vertData[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 
		 1.0f, -1.0f, 1.0f, 0.0f, 
		-1.0f,  1.0f, 0.0f, 1.0f, 
		 1.0f,  1.0f, 1.0f, 1.0f, 
	};
	
	// indices buffer
	uint16_t const indxData[] = {
		0, 1, 2, 1, 3, 2, 0, 0 //Two last zero: padding
	};

	vertBuf = createBuffer(vertData, sizeof(vertData), WGPUBufferUsage_Vertex);
	indxBuf = createBuffer(indxData, sizeof(indxData), WGPUBufferUsage_Index);

	WGPUBindGroupEntry bgTexEntry = {};
	bgTexEntry.binding = 0;
	bgTexEntry.textureView = texView;

	WGPUBindGroupEntry bgSamplerEntry = {};
	bgSamplerEntry.binding = 1;
	bgSamplerEntry.sampler = samplerTex;


	WGPUBindGroupEntry* allBgEntries = new WGPUBindGroupEntry[2];
	allBgEntries[0] = bgTexEntry;
	allBgEntries[1] = bgSamplerEntry;

	WGPUBindGroupDescriptor bgDesc = {};
	bgDesc.layout = bindGroupLayout;
	bgDesc.entryCount = 2;
	bgDesc.entries = allBgEntries;


	bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

	// last bit of clean-up
	wgpuBindGroupLayoutRelease(bindGroupLayout);
}

/**
 * Draws using the above pipeline and buffers.
 */
static bool redraw() {
	WGPUTextureView backBufView = wgpuSwapChainGetCurrentTextureView(swapchain);			// create textureView

	WGPURenderPassColorAttachment colorDesc = {};
	colorDesc.view    = backBufView;
	colorDesc.loadOp  = WGPULoadOp_Clear;
	colorDesc.storeOp = WGPUStoreOp_Store;
	colorDesc.clearColor.r = 0.0f;
	colorDesc.clearColor.g = 0.0f;
	colorDesc.clearColor.b = 0.0f;
	colorDesc.clearColor.a = 1.0f;

	WGPURenderPassDescriptor renderPass = {};
	renderPass.colorAttachmentCount = 1;
	renderPass.colorAttachments = &colorDesc;

	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);			// create encoder
	WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPass);	// create pass

	// #NOTE: You can add updates for the scene in here
	rasterization(teapot, teapotModel, normalTeapot);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	wgpuQueueWriteTexture(queue, &texCopy, img, viewWidth * viewHeight * 4, &texDataLayout, &texSize);	
	

	// draw the triangle (comment these five lines to simply clear the screen)
	wgpuRenderPassEncoderSetPipeline(pass, pipeline);
	wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, 0);
	wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertBuf, 0, 0);
	wgpuRenderPassEncoderSetIndexBuffer(pass, indxBuf, WGPUIndexFormat_Uint16, 0, 0);
	wgpuRenderPassEncoderDrawIndexed(pass, 8, 1, 0, 0, 0);

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

// #TODO: Using these two functions for tasks in the assignment
/**
 * Mouse handling function.
 */
void mouseClickHandler(int button, int action, int x, int y)
{
	printf("button:%d action:%d x:%d y:%d\n", button, action, x, y);
}

/**
 * Keyboard handling function.
 */
void keyPressHandler(int button, int action)
{
	printf("key:%d action:%d\n", button, action);
}


extern "C" int __main__(int /*argc*/, char* /*argv*/[]) {

	beacon.loadOBJ("../data/beacon.obj");
	sphere.loadOBJ("../data/sphere.obj");
	teapot.loadOBJ("../data/teapot.obj");

	calculateViewPortMatrix(viewWidth, viewHeight, Viewport);
	
	//----------- Draw windows and update scene ------------
	if (window::Handle wHnd = window::create(viewWidth, viewHeight, "Hello CS248")) {
		if ((device = webgpu::create(wHnd))) {

			queue = wgpuDeviceGetQueue(device);
			swapchain = webgpu::createSwapChain(device);
			createPipelineAndBuffers(img, viewWidth, viewHeight);

			// bind the user interaction
			window::mouseClicked(mouseClickHandler);
			window::keyPressed(keyPressHandler);

			window::show(wHnd);
			window::loop(wHnd, redraw);


#ifndef __EMSCRIPTEN__
			wgpuBindGroupRelease(bindGroup);
			wgpuBufferRelease(indxBuf);
			wgpuBufferRelease(vertBuf);
			wgpuSamplerRelease(samplerTex);
			wgpuTextureViewRelease(texView);
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
