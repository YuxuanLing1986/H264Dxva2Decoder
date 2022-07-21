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
        IF_FAILED_THROW(m_cNaluParserBuffer.Initialize(NALU_READ_SIZE));
        CH264AtomParser::m_pByteStream = pByteStream;
        CH264AtomParser::m_pByteStream->AddRef();
    }
    catch (HRESULT) {}

    SAFE_RELEASE(pByteStream);

    return hr;
}






//return the pos if found (the patBuf's last byte's pos , not first), else return -1
int64_t BinaryParttenSearch(uint8_t* patBuf, int64_t patBufSz, uint8_t* searchBuf, int64_t searchBufSz)
{
    uint8_t* p = searchBuf;

    while (p < searchBuf + searchBufSz - patBufSz)
    {
        if (memcmp(p, patBuf, patBufSz) == 0)
        {
            return p - searchBuf;
        }
        p++;
    }

    return -1;
}

HRESULT CH2645RawParser::GetNextNaluData(BYTE** ppData, DWORD* pSize)
{


    HRESULT hr = S_OK;
    DWORD bytesRead;
    uint8_t NaluHdr[4] = { 0x00, 0x00, 0x00, 0x01 };
    int32_t NaluHdrSz = sizeof(NaluHdr);
    int64_t nextPos = 0;
    DWORD chunkSz = 0;
    IF_FAILED_RETURN(m_pByteStream->Reset());

    m_pByteStream->Seek(m_lastNalPos);

    //loop here until file or find pos
    m_pByteStream->Read(m_cNaluParserBuffer.GetStartBuffer(), NALU_READ_SIZE, &bytesRead);
    nextPos = BinaryParttenSearch(NaluHdr, NaluHdrSz, m_cNaluParserBuffer.GetStartBuffer() + NaluHdrSz, bytesRead);
    if (nextPos != -1)
    {   
        chunkSz = nextPos + NaluHdrSz;
        m_pByteStream->Seek(m_lastNalPos);
        m_pByteStream->Reset();
        m_cNaluParserBuffer.Reserve(chunkSz);
        m_pByteStream->Read(m_cNaluParserBuffer.GetStartBuffer(), chunkSz, &bytesRead);

        IF_FAILED_RETURN(m_cNaluParserBuffer.SetEndPosition(bytesRead));

        *ppData = m_cNaluParserBuffer.GetStartBuffer();
        *pSize = m_cNaluParserBuffer.GetBufferSize();

        m_lastNalPos += (nextPos + NaluHdrSz);
        m_vPosInfo.push_back(m_lastNalPos);
    }
    else {
        //just assert here , neet to do loop next;
        assert(0);
    }
 

    return hr;
}

HRESULT CH2645RawParser::ParseVideoSPSAndPPS()
{

    HRESULT hr = S_OK;
    BYTE* pData = NULL;
    DWORD dwBufferSize = 0;
    GetNextNaluData(&pData, &dwBufferSize);

    pData += 4;
    // uiForbiddenZeroBit
    BYTE uiForbiddenZeroBit = *pData >> 7;

    if (uiForbiddenZeroBit != 0) {
        TRACE((L"ParseNalHeader : uiForbiddenZeroBit != 0"));
        IF_FAILED_RETURN(E_FAIL);
    }

    // uiNalRefIdc
    m_Picture.btNalRefIdc = *pData >> 5;
    // eNalUnitType
    m_Picture.NalUnitType = (NAL_UNIT_TYPE)(*pData & 0x1f);

    IF_FAILED_RETURN(m_Picture.NalUnitType != NAL_UNIT_SPS ? E_FAIL : S_OK);

    return hr;
}
