package termaudio

import "base:runtime"
import "core:fmt"
import "core:os"

logger :: proc(
	file: string,
	args: ..any,
	sep := "",
	newline: bool = true,
	loc := #caller_location,
) {
	fd, err := os.open(file, os.O_WRONLY | os.O_APPEND)
	if (os.exists(file) == false) {
		fd, err = os.open(file, os.O_WRONLY | os.O_CREATE | os.O_APPEND, 0o644)
	}
	defer os.close(fd)
	for _, i in args {
		switch v in args[i] {
		case:
			fmt.fprint(fd, v, sep)
		}
	}
	if (newline == true) {
		os.write_string(fd, "\n")
	}
}
