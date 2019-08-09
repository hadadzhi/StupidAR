#pragma once

#include "streams.h"

const GUID kMyFilterGuid = { 0x13f5a78d, 0x2331, 0x4edc, { 0xa5, 0xc3, 0xb2, 0xec, 0x95, 0x34, 0x63, 0xbb } }; // {13F5A78D-2331-4EDC-A5C3-B2EC953463BB}
const WCHAR kMyFilterName[] = L"Stupid Audio Renderer";

class StupidAudioRenderer final : public CBaseRenderer {
public:
	StupidAudioRenderer(LPUNKNOWN, HRESULT*);
	~StupidAudioRenderer();

	
	HRESULT CheckMediaType(const CMediaType*) override;
	HRESULT SetMediaType(const CMediaType*) override;
	HRESULT DoRenderSample(IMediaSample*) override;
};
