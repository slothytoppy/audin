package song_queue

import "core:fmt"
import "core:os"
import "core:strings"

file_path_list :: struct {
	base_path: [dynamic]string,
	fullpath:  [dynamic]string,
	count:     u64,
}

@(private)
_read_dir :: proc(path: string, user_data: ^file_path_list) -> (path_arr: file_path_list) {
	assert(user_data != nil)
	assert(os.exists(path))
	fd, _ := os.open(path)
	defer os.close(fd)
	fi, _ := os.read_dir(fd, 1)
	defer delete(fi)
	for i in 0 ..< len(fi) {
		if (fi[i].is_dir) {
			_ = _read_dir(fi[i].fullpath, user_data)
		} else {
			append(&user_data.fullpath, fi[i].fullpath)
			append(&user_data.base_path, fi[i].name)
		}
	}
	user_data.count = cast(u64)len(user_data.fullpath)
	assert(user_data.count > 0)
	return user_data^
}

read_dir :: proc(path: string, path_arr: ^file_path_list, recurse: bool = false) {
	_read_dir(path, path_arr)
}
