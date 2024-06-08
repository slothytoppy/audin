package audio

import "base:builtin"
import "core:fmt"
import "core:os"
import "core:strings"
import "core:testing"

file_path_list :: struct {
	fullpath: [dynamic]string,
}

@(private)
_read_dir :: proc(path: string, user_data: ^file_path_list) -> (path_arr: file_path_list) {
	assert(user_data != nil)
	assert(os.exists(path))
	fd, _ := os.open(path)
	defer os.close(fd)
	fi, _ := os.read_dir(fd, 0)
	defer delete(fi)
	for i in 0 ..< builtin.len(fi) {
		if (fi[i].is_dir) {
			_ = _read_dir(fi[i].fullpath, user_data)
		} else {
			append(&user_data.fullpath, fi[i].fullpath)
		}
	}
	return user_data^
}

read_dir :: proc(path: string, path_arr: ^file_path_list, recurse: bool = false) {
	_read_dir(path, path_arr)
}

len :: proc(q: file_path_list) -> u64 {
	return cast(u64)builtin.len(q.fullpath)
}

base_name :: proc(q: file_path_list, idx: u64) -> string {
	assert(idx <= len(q))
	path := q.fullpath[idx]
	i := strings.last_index(path, "/")
	return path[i + 1:]
}
