#include "MyRenderer.h"

namespace StupidAR {

    MyRenderer::MyRenderer(LPUNKNOWN pUnknown, HRESULT* pResult)
        : CBaseRenderer(kMyFilterGuid, kMyFilterName, pUnknown, pResult) {
    }

    MyRenderer::~MyRenderer() {
    }

    HRESULT MyRenderer::CheckMediaType(const CMediaType*) {
        // Accept anything -- actual checking is done in SetMediaType()
        return S_OK;
    }

    HRESULT MyRenderer::SetMediaType(const CMediaType* pMediaType) {
        return S_OK;
    }

    HRESULT MyRenderer::DoRenderSample(IMediaSample* pSample) {
        return S_OK;
    }

}
