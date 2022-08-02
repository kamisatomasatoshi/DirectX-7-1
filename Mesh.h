#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInput�̃o�[�W�����w��
#include<dinput.h>
#include<d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#include<Windows.h>
#include <tchar.h>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <vector>
#include <string>
#include<DirectXMath.h>

using namespace DirectX;
using namespace std;

class Mesh {

public:
	void Initialize(ID3D12Device* device);

	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	HRESULT result_;

	// �E�B���h�E����
	const int window_width = 1280;
	// �E�B���h�E�c��
	const int window_height = 720;

	HRESULT result;

	// ���_�f�[�^
	XMFLOAT3 vertices[3] = {
	{ -0.5f, -0.5f, +1.0f }, // ����
	{ -0.5f, +0.5f, 0.0f }, // ����
	{ +0.5f, -0.5f, 0.0f }, // �E��
	};

	//�΃v�����X�e�[�g�̐���
	ID3D12PipelineState* pipelineStage = nullptr;
	// ���[�g�V�O�l�`��
	ID3D12RootSignature* rootSignature;

	// ���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};

};