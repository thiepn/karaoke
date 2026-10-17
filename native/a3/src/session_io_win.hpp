#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include "session_export.hpp"
namespace roomtone::local_file {
// A single owner-initiated file. CREATE_NEW provides atomic no-clobber on
// Windows even if a different process creates the chosen path meanwhile.
// Validate before opening the destination; no autosave or background writes.
inline void saveNew(const std::filesystem::path& file,const Handoff& snapshot){
 const auto encoded=session_export::encode(snapshot);
 const HANDLE handle=CreateFileW(file.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,
                                 FILE_ATTRIBUTE_NORMAL,nullptr);
 if(handle==INVALID_HANDLE_VALUE)
  throw std::runtime_error("Session file already exists or cannot be created; choose a NEW .rtk filename");
 DWORD written=0;
 const BOOL complete=WriteFile(handle,encoded.data(),static_cast<DWORD>(encoded.size()),&written,nullptr);
 const BOOL durable=complete&&written==encoded.size()&&FlushFileBuffers(handle);
 CloseHandle(handle);
 if(!durable){
  DeleteFileW(file.c_str());
  throw std::runtime_error("Could not finish owner-selected practice session export");
 }
}
}
#endif
