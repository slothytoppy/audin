package audio_stream

import "core:os"
import "core:sys/linux"
import "core:time"

cmd :: proc(path: string, args: []string, async: bool = false) {
	pid, _ := linux.fork()
	if (pid == 0) {
		os.execvp(path, args)
	}
	if (async) {
		status: u32
		wait_options: linux.Wait_Options = {linux.Wait_Option.WEXITED}
		linux.waitpid(pid, &status, wait_options)
	}
}

rebuild :: proc(path: string) {
	fi, _ := os.stat(path)
	cmd("gcc", []string{"-c", "./deps/dr_mp3.c", "-o", "./deps/dr_mp3.o"})
	cmd("ar", []string{"-rc", "./lib/dr_mp3.a", "./deps/dr_mp3.o"})
	msg: []u8 = {'h', 'e', 'l', 'l', 'o', '\n'}
	os.write(os.stdout, msg)
}
