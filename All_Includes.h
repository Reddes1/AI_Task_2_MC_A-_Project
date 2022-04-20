///////////////////////////////
// Keep Common Includes Here //
///////////////////////////////

//General Includes (Windows Etc)
#include <windows.h>
#include <wrl.h>
#include <stdio.h>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

//XTK Specifics (Update as identified)
#include "DDSTextureLoader.h"

//DX12 Headers (Don't need all, remove selectively later)
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "d3dx12.h"
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "DescriptorHeap.h"
#include "DirectXHelpers.h"
#include "EffectPipelineStateDescription.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "RenderTargetState.h"
#include "ResourceUploadBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"



