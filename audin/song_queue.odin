package termaudio

/*
import "core:c/libc"
import "core:fmt"
import "core:os"
import "core:strings"
import "core:sys/linux"

read_dir :: proc(dir_name: string) -> queue {
	q: queue
	dir, _ := os.open(dir_name)
	flags: linux.Mode
	flags = {.IWUSR, .IRUSR, .IRGRP, .IWGRP, .IROTH}
	if (os.exists("log") != true) {
		linux.creat(cast(cstring)"log", flags)
	}
	out, err := os.open("log", os.O_RDWR)
	if (err != 0) {
		fmt.printf("%s", libc.strerror(cast(i32)err))
		os.exit(1)
	}
	defer os.close(dir)
	defer os.close(out)
	if (err != 0) {
		panic("os.open failed to open a dir")
	}
	fi, er := os.read_dir(dir, 0)

	if (er != 0) {
		fmt.printf("%s", libc.strerror(cast(i32)er))
		os.exit(1)
	}
	dir_path, _ := os.absolute_path_from_relative(dir_name)
	for i := 0; i < len(fi); i += 1 {
		path := strings.builder_make()
		strings.write_string(&path, dir_path)
		strings.write_string(&path, "/")
		strings.write_string(&path, fi[i].name)
		append(&q.songs, strings.to_string(path))
	}
	if (len(q.songs) <= 0) {
		panic("failed to fill queue in read_dir")
	}
	return q
}

set_queue :: proc(q: queue) {
	audio.queue = q
}

get_cursor :: proc() -> u64 {
	return audio.queue.cursor
}

set_cursor :: proc(cursor: u64) {
	audio.queue.cursor = cursor
}

add_queue :: proc(q: queue) {
	for i := 0; i < len(q.songs); i += 1 {
		append(&audio.queue.songs, q.songs[i])
	}
}
*/
