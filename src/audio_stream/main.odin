package audio_stream

import "base:intrinsics"
import "base:runtime"
import "core:c"
import "core:fmt"
import ma "vendor:miniaudio"

data :: struct {
	s:   []u8,
	len: uint,
}

on_open :: proc "cdecl" (
	vfs: ^ma.vfs,
	file_path: cstring,
	open_mode: u32,
	file: ^ma.vfs_file,
) -> ma.result {
	context = runtime.default_context()
	/*
  vfs := vfs
	pfs := cast(^ma.vfs_callbacks)&vfs
  */
	file := file
	da := cast(^data)file^

	return .SUCCESS
}

on_read :: proc "cdecl" (
	vfs: ^ma.vfs,
	file: ma.vfs_file,
	dst: rawptr,
	sizeInBytes: c.size_t,
	pBytesRead: ^c.size_t,
) -> ma.result {
	return .SUCCESS
}

on_seek :: proc "cdecl" (
	pVFS: ^ma.vfs,
	file: ma.vfs_file,
	offset: i64,
	origin: ma.seek_origin,
) -> ma.result {
	return .SUCCESS
}

on_tell :: proc "cdecl" (pVFS: ^ma.vfs, file: ma.vfs_file, pCursor: ^i64) -> ma.result {
	return .SUCCESS
}

on_info :: proc "cdecl" (pVFS: ^ma.vfs, file: ma.vfs_file, pInfo: ^ma.file_info) -> ma.result {
	return .SUCCESS
}

on_close :: proc "cdecl" (pVFS: ^ma.vfs, file: ma.vfs_file) -> ma.result {
	return .SUCCESS
}

on_write :: proc "cdecl" (
	pVFS: ^ma.vfs,
	file: ma.vfs_file,
	pSrc: rawptr,
	sizeInBytes: c.size_t,
	pBytesWritten: ^c.size_t,
) -> ma.result {
	return .SUCCESS
}

on_decode :: proc "cdecl" (
	pDecoder: ^ma.decoder,
	pBufferOut: rawptr,
	bytesToRead: c.size_t,
	pBytesRead: ^c.size_t,
) -> ma.result {
	pBytesRead := pBytesRead
	context = runtime.default_context()
	fmt.println(bytesToRead)
	return .SUCCESS
}


main :: proc() {
	vfs_callbacks: ma.vfs_callbacks = {}
	vfs_callbacks.onOpen = on_open
	vfs_callbacks.onRead = on_read
	vfs_callbacks.onSeek = on_seek
	vfs_callbacks.onTell = on_tell
	vfs_callbacks.onInfo = on_info
	vfs_callbacks.onClose = on_close
	vfs_callbacks.onWrite = on_write
	vfs := cast(^ma.vfs)&vfs_callbacks
	dcfg: ma.decoder_config = ma.decoder_config_init_default()
	dec: ma.decoder
	ma.decoder_init_vfs(vfs, "hello", &dcfg, &dec)

	frame_count_out: u64
	out_frame: rawptr
	frames: u64
	/*
  ma.decoder_init_vfs(vfs, "../../stuff/Disturbed - Who.mp3", &dcfg, &dec)
	dec.onRead = on_decode
  */
	//ma.decoder_init_file("../../stuff/Disturbed - Who.mp3", &dcfg, &dec)
	dec.onRead = on_decode
	if (dec.onRead != nil) {
		dec.onRead(
			&dec,
			&out_frame,
			cast(c.size_t)dec.outputSampleRate * 2,
			cast(^c.size_t)&frame_count_out,
		)
	} else {
		fmt.println("onRead is nil")
	}
	ma.decode_from_vfs(vfs, "../../stuff/Disturbed - Who.mp3", &dcfg, &frame_count_out, &out_frame)
}
