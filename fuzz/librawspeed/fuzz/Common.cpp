/*
    RawSpeed - RAW file decoder.

    Copyright (C) 2017 Roman Lebedev

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "fuzz/Common.h"               // for CreateRawImage
#include "common/Point.h"              // for iPoint2D
#include "common/RawImage.h"           // for RawImage
#include "common/RawspeedException.h"  // for RawspeedException
#include "io/ByteStream.h"             // for ByteStream
#include "metadata/ColorFilterArray.h" // for ColorFilterArray
#include <limits>                      // for numeric_limits

rawspeed::RawImage CreateRawImage(rawspeed::ByteStream* bs) {
  assert(bs);

  const rawspeed::uint32 width = bs->getU32();
  const rawspeed::uint32 height = bs->getU32();
  const rawspeed::uint32 type = bs->getU32();
  const rawspeed::uint32 cpp = bs->getU32();
  const rawspeed::uint32 isCFA = bs->getU32();

  if (type != rawspeed::TYPE_USHORT16 && type != rawspeed::TYPE_FLOAT32)
    ThrowRSE("Unknown image type: %u", type);

  rawspeed::RawImage mRaw(
      rawspeed::RawImage::create(rawspeed::RawImageType(type)));

  mRaw->dim = rawspeed::iPoint2D(width, height);
  mRaw->setCpp(cpp);
  mRaw->isCFA = isCFA;

  return mRaw;
};

rawspeed::ColorFilterArray CreateCFA(rawspeed::ByteStream* bs) {
  assert(bs);

  const rawspeed::uint32 cfaWidth = bs->getU32();
  const rawspeed::uint32 cfaHeight = bs->getU32();

  rawspeed::ColorFilterArray cfa;

  if (cfaHeight &&
      cfaWidth > std::numeric_limits<decltype(cfaWidth)>::max() / cfaHeight)
    ThrowIOE("Integer overflow when calculating CFA area");
  bs->check(cfaWidth * cfaHeight, 4);

  cfa.setSize(rawspeed::iPoint2D(cfaWidth, cfaHeight));

  for (auto x = 0U; x < cfaWidth; x++) {
    for (auto y = 0U; y < cfaHeight; y++) {
      const rawspeed::uint32 color = bs->getU32();
      if (color < rawspeed::CFA_BEGIN || color > rawspeed::CFA_END)
        ThrowRSE("Unknown color: %u", color);

      cfa.setColorAt(rawspeed::iPoint2D(x, y), rawspeed::CFAColor(color));
    }
  }

  return cfa;
};