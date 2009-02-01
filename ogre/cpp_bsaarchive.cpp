/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.snaptoad.com/

  This file (cpp_bsaarchive.cpp) is part of the OpenMW package.

  OpenMW is distributed as free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  version 3, as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  version 3 along with this program. If not, see
  http://www.gnu.org/licenses/ .

 */

// This file inserts an archive manager for .bsa files into the ogre
// resource loading system. It uses callbacks to D to interact with
// the D-based bsa handling code. The D bindings are in
// core/resource.d.

// Callbacks to D code

// Does the file exist in the archives?
extern "C" int32_t d_bsaExists(const char *filename);

// Open a file. Return the pointer and size.
extern "C" void d_bsaOpenFile(const char *filename,
                              void **retPtr, uint32_t *retSize);


// This archive does not cover one .bsa file, instead it interacts
// with the all the loaded bsas and treates them as one archive.
class BSAArchive : public Archive
{
public:

  BSAArchive(const String& name)
             : Archive(name, "BSA") {}
  ~BSAArchive() {}

  bool isCaseSensitive(void) const { return false; }

  // The archives are already loaded in D code, and they are never
  // unloaded. Just ignore these.
  void load() {}
  void unload() {}

  // Open a file in the archive. We delegate the function to D-code.
  DataStreamPtr open(const String& filename) const
  {
    //std::cout << "open(" << filename << ")\n";
    void *ptr;
    uint32_t size;

    // Open the file. The BSA archives are memory mapped, D code
    // returns a pointer and a file size.
    d_bsaOpenFile(filename.c_str(), &ptr, &size);

    return DataStreamPtr(new MemoryDataStream(ptr, size));
  }

  // This is never called as far as I can see.
  StringVectorPtr list(bool recursive = true, bool dirs = false)
  {
    std::cout << "list(" << recursive << ", " << dirs << ")\n";
    StringVectorPtr ptr = StringVectorPtr(new StringVector());
    return ptr;
  }
  // Also never called.
  FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false)
  {
    std::cout << "listFileInfo(" << recursive << ", " << dirs << ")\n";
    FileInfoListPtr ptr = FileInfoListPtr(new FileInfoList());
    return ptr;
  }

  time_t getModifiedTime(const String&) { return 0; }

  // After load() is called, find("*") is called once. It doesn't seem
  // to matter that we return an empty list, exists() gets called on
  // the correct files anyway.
  StringVectorPtr find(const String& pattern, bool recursive = true,
                       bool dirs = false)
  {
    //std::cout << "find(" << pattern << ", " << recursive
    //          << ", " << dirs << ")\n";
    StringVectorPtr ptr = StringVectorPtr(new StringVector());
    return ptr;
  }

  // Gets called once for each of the ogre formats, *.program,
  // *.material etc. It's also called by MyGUI to find textures, so we
  // have to channel it through exists().
  FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true,
                               bool dirs = false)
  {
    /*
    std::cout << "findFileInfo(" << pattern << ", " << recursive
              << ", " << dirs << ")\n";
    */

    FileInfoListPtr ptr = FileInfoListPtr(new FileInfoList());

    // Check if the file exists (only works for single files - wild
    // cards and recursive search isn't implemented.)
    if(exists(pattern))
      {
        FileInfo fi;
        fi.archive = this;
        fi.filename = pattern;
        fi.path = "";
        // It apparently doesn't matter that we return bogus
        // information
        fi.compressedSize = fi.uncompressedSize = 0;

        ptr->push_back(fi);
      }

    return ptr;
  }

  // Check if the file exists.
  bool exists(const String& filename)
  {
    //std::cout << "exists(" << filename << ")\n";
    return d_bsaExists(filename.c_str()) != 0;
  }
};

// An archive factory for BSA archives
class BSAArchiveFactory : public ArchiveFactory
{
public:
  virtual ~BSAArchiveFactory() {}

  const String& getType() const
  {
    static String name = "BSA";
    return name;
  }

  Archive *createInstance( const String& name ) 
  {
    return new BSAArchive(name);
  }

  void destroyInstance( Archive* arch) { delete arch; }
};

BSAArchiveFactory mBSAFactory;
