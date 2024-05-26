package termaudio

import "core:os"
import "core:strings"

logger :: proc(data: string) {
	fd, _ := os.open("log", os.O_WRONLY | os.O_APPEND)
	os.write_string(fd, data)
	if (!strings.has_suffix(data, "\n")) {
		os.write_byte(fd, '\n')
	}
}
