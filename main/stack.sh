#values=$(grep -wnH "\b40......\b" stack.txt)
#echo $values
awk '{
    while (match($0, / 40....../)) {
        pattern = substr($0, RSTART, RLENGTH);
		system("/C/Espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-addr2line -aipfC -e out/build/app.out " pattern);
        $0 = substr($0, RSTART + RLENGTH);
    }
}' stack.txt