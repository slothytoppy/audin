package song_queue

import "../id3"
import "core:fmt"
import "core:mem"
import "core:os"
import "core:path/filepath"
import "core:strings"
import "core:sys/linux"

file_path_list :: struct {
	base_path: string,
	files:     [dynamic]string,
	count:     u64,
	cur_dir:   string,
}

@(private)
walk_proc :: proc(
	fi: os.File_Info,
	in_err: os.Errno,
	user_data: rawptr,
) -> (
	out_err: os.Errno,
	skip_dir: bool,
) {
	user_data := cast(^file_path_list)user_data
	if (fi.is_dir) {
		user_data.cur_dir = fi.name
		return
	}
	at_root := false
	if (strings.compare(
			   user_data.base_path[len(user_data.base_path) - len(user_data.cur_dir):],
			   user_data.cur_dir,
		   ) ==
		   0) {
		at_root = true
	}
	if (strings.has_suffix(user_data.base_path, user_data.cur_dir) == true) {
		fmt.println(true)
	}
	/*
	idx := strings.last_index_byte(user_data.cur_dir, '/')
	buf := user_data.cur_dir[idx:]
	at_root: bool = false
	if (strings.compare(user_data.cur_dir, buf) == 0) {
		at_root = true
	}
  if(strings.has_suffix(user_data.base_path, "/")){}
  */
	append(
		&user_data.files,
		strings.concatenate(
			 {
				strings.has_suffix(user_data.base_path, "/") \
				? user_data.base_path \
				: strings.concatenate({user_data.base_path, "/", user_data.cur_dir, "/"}),
				fi.name,
			},
		),
	)
	return os.ERROR_NONE, false
}

@(private)
@(require_results)
_read_dir :: proc(path: string, user_data: ^file_path_list) -> (path_arr: file_path_list) {
	fd, _ := os.open(path)
	defer os.close(fd)
	fi, _ := os.read_dir(fd, -1)
	buf: string = ""
	for i := 0; i < len(fi); i += 1 {
		if (fi[i].is_dir) {
			_ = _read_dir(fi[i].fullpath, user_data)
		} else {
			buf = strings.concatenate({fi[i].fullpath})
			append(&user_data.files, buf)
		}
		fmt.println("file:", user_data.files[:])
	}
	user_data.count = cast(u64)len(user_data.files)
	return user_data^
}

@(require_results)
read_dir :: proc(path: string, recurse: bool = false) -> (path_arr: file_path_list) {
	return _read_dir(path, &path_arr)
}
