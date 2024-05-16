/*
Copyright (c) 2019, Wolfgang Ruppel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*! \file    AS_02_ProRes.cpp
  \version $Id$       
  \brief   AS-02 library, ProRes essence reader and writer implementation
*/

#include "AS_02_internal.h"

#include <map>
#include <iostream>
#include <iomanip>


//------------------------------------------------------------------------------------------

static std::string ProRes_PACKAGE_LABEL = "File Package: RDD 44 frame wrapping of ProRes video";
static std::string PICT_DEF_LABEL = "Image Track";


//------------------------------------------------------------------------------------------


class AS_02::ProRes::MXFReader::h__Reader : public AS_02::h__AS02Reader
{
  ASDCP_NO_COPY_CONSTRUCT(h__Reader);

public:
  h__Reader(const Dictionary* d, const Kumu::IFileReaderFactory& fileReaderFactory) :
    AS_02::h__AS02Reader(d, fileReaderFactory) {}

  virtual ~h__Reader() {}

  ASDCP::Result_t    OpenRead(const std::string&);
  ASDCP::Result_t    ReadFrame(ui32_t, ASDCP::JP2K::FrameBuffer&, ASDCP::AESDecContext*, ASDCP::HMACContext*);
};

// TODO: This will ignore any body partitions past the first
//
//
ASDCP::Result_t
AS_02::ProRes::MXFReader::h__Reader::OpenRead(const std::string& filename)
{
	  Result_t result = OpenMXFRead(filename);

	  if( KM_SUCCESS(result) )
	    {
	      InterchangeObject* tmp_iobj = 0;

	      m_HeaderPart.GetMDObjectByType(OBJ_TYPE_ARGS(CDCIEssenceDescriptor), &tmp_iobj);

	      if ( tmp_iobj == 0 )
		{
		  DefaultLogSink().Error("CDCIEssenceDescriptor not found.\n");
		}

	      std::list<InterchangeObject*> ObjectList;
	      m_HeaderPart.GetMDObjectsByType(OBJ_TYPE_ARGS(Track), ObjectList);

	      if ( ObjectList.empty() )
		{
		  DefaultLogSink().Error("MXF Metadata contains no Track Sets.\n");
		  return RESULT_AS02_FORMAT;
		}
	    }

	  return result;
}
//
//
Result_t
AS_02::ProRes::MXFReader::h__Reader::ReadFrame(ui32_t FrameNum, ASDCP::JP2K::FrameBuffer& FrameBuf,
		      ASDCP::AESDecContext* Ctx, ASDCP::HMACContext* HMAC)
{
  if ( ! m_File->IsOpen() )
    return RESULT_INIT;

  assert(m_Dict);
  return ReadEKLVFrame(FrameNum, FrameBuf, m_Dict->ul(MDD_FrameWrappedProResPictureElement), Ctx, HMAC);
}



AS_02::ProRes::MXFReader::MXFReader(const Kumu::IFileReaderFactory& fileReaderFactory)
{
  m_Reader = new h__Reader(&DefaultCompositeDict(), fileReaderFactory);
}

AS_02::ProRes::MXFReader::~MXFReader()
{
}

// Warning: direct manipulation of MXF structures can interfere
// with the normal operation of the wrapper.  Caveat emptor!
//
ASDCP::MXF::OP1aHeader&
AS_02::ProRes::MXFReader::OP1aHeader()
{
  if ( m_Reader.empty() )
    {
      assert(g_OP1aHeader);
      return *g_OP1aHeader;
    }

  return m_Reader->m_HeaderPart;
}

// Warning: direct manipulation of MXF structures can interfere
// with the normal operation of the wrapper.  Caveat emptor!
//
AS_02::MXF::AS02IndexReader&
AS_02::ProRes::MXFReader::AS02IndexReader()
{
  if ( m_Reader.empty() )
    {
      assert(g_AS02IndexReader);
      return *g_AS02IndexReader;
    }

  return m_Reader->m_IndexAccess;
}

// Warning: direct manipulation of MXF structures can interfere
// with the normal operation of the wrapper.  Caveat emptor!
//
ASDCP::MXF::RIP&
AS_02::ProRes::MXFReader::RIP()
{
  if ( m_Reader.empty() )
    {
      assert(g_RIP);
      return *g_RIP;
    }

  return m_Reader->m_RIP;
}

// Open the file for reading. The file must exist. Returns error if the
// operation cannot be completed.
ASDCP::Result_t
AS_02::ProRes::MXFReader::OpenRead(const std::string& filename) const
{
  return m_Reader->OpenRead(filename);
}

//
Result_t
AS_02::ProRes::MXFReader::Close() const
{
  if ( m_Reader && m_Reader->m_File->IsOpen() )
    {
      m_Reader->Close();
      return RESULT_OK;
    }

  return RESULT_INIT;
}

//
Result_t
AS_02::ProRes::MXFReader::ReadFrame(ui32_t FrameNum, ASDCP::JP2K::FrameBuffer& FrameBuf,
					   ASDCP::AESDecContext* Ctx, ASDCP::HMACContext* HMAC) const
{
  if ( m_Reader && m_Reader->m_File->IsOpen() )
    return m_Reader->ReadFrame(FrameNum, FrameBuf, Ctx, HMAC);

  return RESULT_INIT;
}



// Fill the struct with the values from the file's header.
// Returns RESULT_INIT if the file is not open.
ASDCP::Result_t
AS_02::ProRes::MXFReader::FillWriterInfo(WriterInfo& Info) const
{
  if ( m_Reader && m_Reader->m_File->IsOpen() )
    {
      Info = m_Reader->m_Info;
      return RESULT_OK;
    }

  return RESULT_INIT;
}

//
void
AS_02::ProRes::MXFReader::DumpHeaderMetadata(FILE* stream) const
{
  if ( m_Reader && m_Reader->m_File->IsOpen() )
    {
      m_Reader->m_HeaderPart.Dump(stream);
    }
}

//
void
AS_02::ProRes::MXFReader::DumpIndex(FILE* stream) const
{
  if ( m_Reader && m_Reader->m_File->IsOpen() )
    {
      m_Reader->m_IndexAccess.Dump(stream);
    }
}



//
// end AS_02_ProRes.cpp
//

