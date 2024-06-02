package song_queue

import "core:os"
import "core:strings"

file_path_list :: struct {
	base_path: [dynamic]string,
	files:     [dynamic]string,
	count:     u64,
	cur_dir:   string,
}

@(private)
@(require_results)
_read_dir :: proc(path: string, user_data: ^file_path_list) -> (path_arr: file_path_list) {
	assert(user_data != nil)
	assert(os.exists(path))
	fd, _ := os.open(path)
	defer os.close(fd)
	fi, _ := os.read_dir(fd, -1)
	for i := 0; i < len(fi); i += 1 {
		if (fi[i].is_dir) {
			_ = _read_dir(fi[i].fullpath, user_data)
		} else {
			append(&user_data.files, strings.concatenate({fi[i].fullpath}))
			append(&user_data.base_path, fi[i].name)
		}
	}
	user_data.count = cast(u64)len(user_data.files)
	assert(user_data.count > 0)
	return user_data^
}

@(require_results)
read_dir :: proc(path: string, recurse: bool = false) -> (path_arr: file_path_list) {
	return _read_dir(path, &path_arr)
}
