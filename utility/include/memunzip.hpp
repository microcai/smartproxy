
#pragma once

#include <map>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "minizip/unzip.h"

extern const char _binary_site_zip_start[];
extern const char _binary_site_zip_end[];

namespace{

	struct mem_zip_file
	{
		const voidpf base_ptr;
		ZPOS64_T offset;
		uLong fsize;
		int error;

		mem_zip_file(const voidpf _base, uLong _size)
			: base_ptr(_base)
			, offset(0)
			, fsize(_size)
			, error(0)
		{}
	};

	static void memzip_init_zlib_filefunc(zlib_filefunc64_def& file_ops, const void* zip_content, uLong zip_size)
	{
		file_ops.opaque =  new mem_zip_file((const voidpf)zip_content, zip_size);

		file_ops.zopen64_file = [](voidpf opaque, const void* filename, int mode)
		{
			// return membaseptr as filestream !
			return opaque;
		};

		file_ops.zread_file = [](voidpf opaque, voidpf stream, void* buf, uLong size) -> uLong
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			if (f->error == 0)
			{
				uLong copyable_size = std::min(size, uLong(f->fsize - f->offset));
				if (copyable_size > 0)
				{
					std::memcpy(buf, reinterpret_cast<char*>(f->base_ptr) + f->offset, copyable_size);
					f->offset += copyable_size;
					return copyable_size;
				}
				f->error = EOF;
				return 0;
			}
			return -1;
		};

		file_ops.zwrite_file = [](voidpf opaque, voidpf stream, const void* buf, uLong size) -> uLong
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			return -1;
		};

		file_ops.ztell64_file = [](voidpf opaque, voidpf stream) -> ZPOS64_T
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			return f->offset;
		};

		file_ops.zseek64_file = [](voidpf opaque, voidpf stream, ZPOS64_T offset, int origin) -> long
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			switch (origin)
			{
			case ZLIB_FILEFUNC_SEEK_CUR:
				f->offset += offset;
				break;
			case ZLIB_FILEFUNC_SEEK_END:
				f->offset = f->fsize - offset;
				break;
			case ZLIB_FILEFUNC_SEEK_SET:
				f->offset = offset;
				break;
			default:
				return -1;
			}

			return 0;
		};

		file_ops.zclose_file = [](voidpf opaque, voidpf stream) -> int
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			boost::checked_delete(f);
			return 0;
		};

		file_ops.zerror_file = [](voidpf opaque, voidpf stream) -> int
		{
			mem_zip_file * f = reinterpret_cast<mem_zip_file*>(stream);
			return f->error;
		};
	}
}
