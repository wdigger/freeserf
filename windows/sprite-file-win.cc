/*
 * sprite-file-sdl.cc - Sprite loaded from file implementation
 *
 * Copyright (C) 2019  Wicked_Digger <wicked_digger@mail.ru>
 *
 * This file is part of freeserf.
 *
 * freeserf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * freeserf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freeserf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/sprite-file.h"

#include <windows.h>
#include <gdiplus.h>
#include <codecvt>

ULONG_PTR gdiplusToken;

SpriteFile::SpriteFile() {
  /* Initialize Video subsystem */
  Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0;
  Gdiplus::GdiplusStartupOutput gdiplusStartupOutput = { 0 };
  if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput,
    &gdiplusStartupOutput) != Gdiplus::Ok) {
    Log::Error["video-win"] << "Unable to initialize Gdiplus.";
    return;
  }
}

SpriteFile::~SpriteFile() {
  Gdiplus::GdiplusShutdown(gdiplusToken);
  gdiplusToken = 0;
}

bool
SpriteFile::load(const std::string &path) {
  wchar_t wpath[MAX_PATH];
  size_t l = mbstowcs(wpath, path.c_str(), MAX_PATH);
  Gdiplus::Bitmap *image = Gdiplus::Bitmap::FromFile(wpath);
  if (image == nullptr) {
    return false;
  }
  width = image->GetWidth();
  height = image->GetHeight();

  Gdiplus::Rect r(0, 0, (INT)width, (INT)height);
  Gdiplus::BitmapData bits;
  image->LockBits(&r, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bits);
  size_t size = width * height * 4;
  data = reinterpret_cast<uint8_t*>(malloc(size));
  bool res = (bits.Scan0 != nullptr) && (data != nullptr);
  if (res) {
    memcpy(data, bits.Scan0, size);
  }
  image->UnlockBits(&bits);
  return res;
}
