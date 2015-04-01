function base_dir(f)
{
	ne = split(f, s, "/")
	for(k=ne;k>=1 && s[k] != "src";k--);
	r = s[k]
	for(j=k+1;j<ne;j++)
		r = r "/" s[j]
	return r
}

{
	for(i=1; i<=NF; i++) {
		if($i != "\\") {
			ff = $i
			fd = base_dir(ff)
			if(substr(ff,length(ff)) == ":") {
				root_dir = fd
			} else if(fd != root_dir) {
				if(fd == "src/emu" || fd == "src/osd" || fd == "src/lib/util")
					$i = ""
			}
		}
	}
	print
}