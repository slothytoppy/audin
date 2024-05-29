package termaudio

import "core:fmt"
import "core:os"
import "core:strings"

main :: proc() {
	curr_dir := os.get_current_directory()
	target_dir := "/bin"
	arg_path, out_path: string
	args: []string

	if (strings.has_suffix(curr_dir, "audin")) {
		arg_path = "src"
		out_path = "-out:bin/audin"
	} else if (strings.has_suffix(curr_dir, "src")) {
		arg_path = "."
		os.set_current_directory("../")
		out_paths: []string = {"-out:", os.get_current_directory(), "/bin/audin"}
		out_path, _ = strings.concatenate(out_paths)
	} else {
		fmt.panicf("script %s is not in src or root of the project", #file)
	}
	pid, _ := os.fork()
	if (pid > 0) {
		args = {"odin", "build", arg_path, out_path}
		err := os.execvp(args[0], args[1:])
		if (err != os.ERROR_NONE) {
			fmt.print(os.get_last_error())
		}
	}
}
