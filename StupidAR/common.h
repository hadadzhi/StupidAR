#pragma once

#define RETURN_FAILED(hr_expr) { HRESULT hr = (hr_expr); if (FAILED(hr)) return hr; }
