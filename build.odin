package build

import "base:runtime"
import "core:fmt"
import "core:os"
import "core:strings"
import "core:sys/linux"
import "core:thread"

cmd_info :: struct {
	args: []string,
}

@(private)
_cmd :: proc(t: ^thread.Thread) {
	args := transmute(^cmd_info)t.data
	fmt.println(args.args)
	pid, _ := os.fork()
	if (pid == 0) {
		os.execvp(args.args[0], args.args[1:])
	}
	if (strings.has_prefix(args.args[0], "./")) {
		status: u32
		wait_options: linux.Wait_Options = {}
		linux.waitpid(cast(linux.Pid)pid, &status, wait_options)
	}
	return
}

cmd :: proc(args: []string) {
	threads := make([dynamic]^thread.Thread, 0, 2)
	defer delete(threads)

	if t := thread.create(_cmd); t != nil {
		t.init_context = context
		t.user_index = len(threads)
		cmd_info: cmd_info
		cmd_info.args = args
		t.data = cast(rawptr)&cmd_info
		append(&threads, t)
		thread.start(t)
	}

	for len(threads) > 0 {
		for i := 0; i < len(threads);  /**/{
			if t := threads[i]; thread.is_done(t) {
				fmt.printf("Thread %d is done\n", t.user_index)
				thread.destroy(t)
				ordered_remove(&threads, i)
			} else {
				i += 1
			}
		}
	}
}

main :: proc() {
	cmd({"odin", "build", "./src", "-out:./bin/audin"})
	cmd({"./bin/audin"})
	return
}
