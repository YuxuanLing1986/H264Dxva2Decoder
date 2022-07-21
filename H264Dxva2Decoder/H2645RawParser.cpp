//----------------------------------------------------------------------------------------------
// H2645RawParser.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"


HRESULT CH2645RawParser::Initialize(LPCWSTR wszFile) {

    HRESULT hr;

    IF_FAILED_RETURN(wszFile ? S_OK : E_INVALIDARG);
    IF_FAILED_RETURN(CH264AtomParser::m_pByteStream ? ERROR_ALREADY_INITIALIZED : S_OK);

    CMFByteStream* pByteStream = NULL;

    try {

        IF_FAILED_THROW(CMFByteStream::CreateInstance(&pByteStream));
        IF_FAILED_THROW(pByteStream->Initialize(wszFile));

        CH264AtomParser::m_pByteStream = pByteStream;
        CH264AtomParser::m_pByteStream->AddRef();
    }
    catch (HRESULT) {}

    SAFE_RELEASE(pByteStream);

    return hr;
}


HRESULT CH2645RawParser::GetNextNaluData(BYTE** ppData, DWORD* pSize)
{


    HRESULT hr;
    IF_FAILED_RETURN(m_pByteStream->Reset());





    return hr;
}