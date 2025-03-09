#!/bin/bash

# /usr/lib

list_file=list.txt
cmd_file=tmp.sh

solved=()
solved_len=0

function solve_file_dep(){
	local file=$(realpath "$1")
	local real_file="$2"
	echo solving "\"$real_file\"" ...
	echo "$real_file" >> "$list_file"
	local filename=$(basename "$file")
	local framework_dir="$3"
	mkdir -p "$framework_dir"
	framework_dir=$(realpath "$framework_dir" | sed "s:/$::")
	local exe_file=$(realpath "$4")
	local main_exe="$5"
	local reg_pattern="$6"
	local exe_path=$(realpath $(dirname "$real_file") | sed "s:/$::")
	local raw_file_path=$(realpath $(dirname "$file") | sed "s:/$::")
	# local exe_path=$(realpath $(dirname "$exe_file") | sed "s:/$::")
	local relative_path=$(realpath --relative-to="$exe_path" "$framework_dir" | sed "s:/$::" | sed 's|^\./||')
	local relative_path_with_symbol
	if [ $main_exe -eq 1 ];	then
		relative_path_with_symbol=$(echo "@executable_path/$relative_path" | sed 's|/\.$||')
	else
		relative_path_with_symbol=$(echo "@loader_path/$relative_path" | sed 's|/\.$||')
	fi
	shift 6
	
	local rpath=($(otool -l $file 2> /dev/null | grep -A 2 "LC_RPATH" | grep "^ *path.*" | sed "s/\b*/ /g" | awk '{print $2}' | grep -v "$reg_pattern" | sed "s:/$::"))	
	# echo ${rpath[@]}
	local dep_file=($(otool -L $file 2> /dev/null | tail -n +2 | awk '{print $1}' | grep -v "$reg_pattern" | sed "s:/$::"))
	local len=${#dep_file[@]}
	local i
	local raw_path=()
	local framework_files=()
	local relative_files=()
	local need_resource=()
	local raw_res=()
	local new_res=()
	for ((i=0; i<$len; i++))
	do
		local t_file="${dep_file[$i]}"
		t_file=$(echo "$t_file" | sed "s|@loader_path|$raw_file_path|g")
		# echo $t_file
		local tmp_file_name=$(basename "$t_file")
		local no_match=1
		local pattern
		for pattern in "$@"
		do
			local match_ret=$(echo "$t_file" | grep "$pattern")
			if [ "$match_ret" != "" ];	then
				no_match=0
				break
			fi
		done
		if [ $# -eq 0 ];	then
			no_match=0;
		fi
		if 	[ $no_match -eq 1 ];	then
		# if [ $no_match -eq 1 ] || [ "$tmp_file_name" == "$filename" ];	then
			unset dep_file[$i]
			continue
		fi
		local match=$(echo "$t_file" | grep -o "^@rpath")
		if [ "$match" == "@rpath" ];	then
			t_file=$(echo "$t_file" | sed "s/^@rpath\/\?//g")
			local j
			local first=1
			for j in "${rpath[@]}"
			do
				local tmp_file="$j/$t_file"
				tmp_file=$(echo "$tmp_file" | sed "s|@loader_path|$raw_file_path|g")
				# echo $tmp_file
				if [ -f "$tmp_file" ];	then
					tmp_file=$(realpath "$tmp_file")
					local dir_name="$(dirname "$tmp_file")"
					raw_res[$i]="$dir_name/Resources"
					new_res[$i]="$framework_dir/$tmp_file_name"
					if [ -d "${raw_res[$i]}" ];	then
						need_resource[$i]=1
						framework_files[$i]="$framework_dir/$tmp_file_name/$tmp_file_name"
						relative_files[$i]="$relative_path_with_symbol/$tmp_file_name/$tmp_file_name"
					else
						need_resource[$i]=0
						framework_files[$i]="$framework_dir/$tmp_file_name"
						relative_files[$i]="$relative_path_with_symbol/$tmp_file_name"
					fi
					raw_path[$i]="${dep_file[$i]}"
					dep_file[$i]="$tmp_file"
					first=0
					break
				fi
			done
			if [ $first -eq 1 ];	then
				unset dep_file[$i]
			fi
		else
			if [ -f "$t_file" ];	then
				local dir_name="$(dirname "$t_file")"
				raw_res[$i]="$dir_name/Resources"
				new_res[$i]="$framework_dir/$tmp_file_name"
				if [ -d "${raw_res[$i]}" ];	then
					need_resource[$i]=1
					framework_files[$i]="$framework_dir/$tmp_file_name/$tmp_file_name"
					relative_files[$i]="$relative_path_with_symbol/$tmp_file_name/$tmp_file_name"
				else
					need_resource[$i]=0
					framework_files[$i]="$framework_dir/$tmp_file_name"
					relative_files[$i]="$relative_path_with_symbol/$tmp_file_name"
				fi
				raw_path[$i]="${dep_file[$i]}"
				dep_file[$i]="$t_file"
			else
				unset dep_file[$i]
			fi
		fi
	done
	dep_file=(${dep_file[@]})
	raw_path=(${raw_path[@]})
	framework_files=(${framework_files[@]})
	relative_files=(${relative_files[@]})
	need_resource=(${need_resource[@]})
	raw_res=(${raw_res[@]})
	new_res=(${new_res[@]})
	for ((i=0; i<${#dep_file[@]}; i++))
	do
		if [ ${need_resource[$i]} -eq 1 ];	then
			echo install_name_tool -change "${raw_path[$i]}" "${relative_files[$i]}" "$real_file" " 2> /dev/null" >> "$cmd_file"
			echo mkdir "-p" "${new_res[$i]}" >> "$cmd_file"
			if [ "${raw_res[$i]}" != "${new_res[$i]}" ];	then
				echo if "[ ! -f \"${new_res[$i]}\" ];"	then >> "$cmd_file"
				echo "\tcp" "-rf" "${raw_res[$i]}" "${new_res[$i]}" >> "$cmd_file"
				echo fi >> "$cmd_file"
			fi
			if [ "${dep_file[$i]}" != "${framework_files[$i]}" ];	then
				echo if "[ ! -f \"${framework_files[$i]}\" ];"	then >> "$cmd_file"
				echo "\tcp" "${dep_file[$i]}" "${framework_files[$i]}" >> "$cmd_file"
				echo fi >> "$cmd_file"
			fi
		else
			echo install_name_tool -change "${raw_path[$i]}" "${relative_files[$i]}" "$real_file" " 2> /dev/null" >> "$cmd_file"
			if [ "${dep_file[$i]}" != "${framework_files[$i]}" ];	then
				echo if "[ ! -f \"${framework_files[$i]}\" ];"	then >> "$cmd_file"
				echo "\tcp" "${dep_file[$i]}" "${framework_files[$i]}" >> "$cmd_file"
				echo fi >> "$cmd_file"
			fi
		fi
	done
	for ((i=0; i<${#dep_file[@]}; i++))
	do
		find=0
		local j
		for j in "${solved[@]}"
		do
			if [ "$j" == "${framework_files[$i]}" ];	then
				find=1
			fi
		done
		if [ $find -eq 0 ];	then
			solved[$solved_len]="${framework_files[$i]}"
			solved_len=$((solved_len+1))
			# echo solve_file_dep "${dep_file[$i]}" "${framework_files[$i]}" "$framework_dir" "$exe_file" "0" "$reg_pattern" "$@"
			solve_file_dep "${dep_file[$i]}" "${framework_files[$i]}" "$framework_dir" "$exe_file" "0" "$reg_pattern" "$@"
		fi
	done
}

function solve_dir() {
	local current_dir="$1"
	local raw_qml_path="$2"
	local global_framework="$3"
	shift 3
	local files=($(ls "$current_dir"))
	local i
	for i in "${files[@]}"
	do
		local tmp_file="$current_dir/$i"
		if [ -f "$tmp_file" ];	then
			local match=$(echo "$tmp_file" | grep "\.dylib$")
			if [ "$match" != "" ];	then
				solve_file_dep "$raw_qml_path/$i" "$tmp_file" "$global_framework" "$tmp_file" "$@"
			fi
		else
			if [ -d "$tmp_file" ];	then
				solve_dir "$tmp_file" "$raw_qml_path/$i" "$global_framework" "$@"
			fi
		fi
	done
}

if [ -f "$cmd_file" ];	then
	rm "$cmd_file"
fi

if [ -f "$list_file" ];	then
	rm "$list_file"
fi

solved_len=1
solved[0]=$(realpath "$1")
global_framework="$2"
global_reg_pattern="$3"
shift 3

# solve resource
content_path=$(realpath $(dirname "${solved[0]}")/..)
res_path="$content_path/Resources"
raw_qml_path="/Users/kazefx/Qt/6.8.2/macos"
solve_dir "$res_path" "$raw_qml_path" "$global_framework" "0" "$global_reg_pattern" "$@"

solve_file_dep "${solved[0]}" "${solved[0]}" "$global_framework" "${solved[0]}" "1" "$global_reg_pattern" "$@"

