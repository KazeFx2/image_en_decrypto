cmd_file=tmp.sh
cp_cmds=($(cat "$cmd_file" | grep "^cp " | sort -u | tr ' ' '$'))
int_cmds=($(cat "$cmd_file" | grep "^install_name_tool " | sort -u | tr ' ' '$'))
mkdir_cmds=($(cat "$cmd_file" | grep "^mkdir " | sort -u | tr ' ' '$'))
cp_files=()
int_files=()
mkdir_files=()
cp_sh=cp.sh
int_sh=int.sh
mkdir_sh=mkdir.sh
# cat "$cmd_file" | grep "^install_name_tool " | sort -u > $int_sh

if [ $1 -eq 1 ] || [ "$1" == "" ];   then
    if [ -f "$mkdir_sh" ];  then
        rm $mkdir_sh
    fi
    for((i=0;i<${#mkdir_cmds[@]};i++))
    do
        tmp=${mkdir_cmds[$i]}
        tmp=$(echo $tmp | tr '$' ' ')
        tmp_array=($tmp)
        len=${#tmp_array[@]}
        to=$((len-1))
        echo \[$((i+1))/${#mkdir_cmds[@]}\] solving mkdir cmd "\"$tmp\"" ...
        mkdir_files[$i]="echo\$\\[$((i+1))/${#mkdir_cmds[@]}\\]\$mkdiring\$\"${tmp_array[$to]}\" ${mkdir_cmds[$i]}"
        echo ${mkdir_files[$i]} | tr ' ' '\n' | tr '$' ' ' >> $mkdir_sh
    done
fi

if [ $1 -eq 2 ] || [ "$1" == "" ];    then
    if [ -f "$cp_sh" ];  then
        rm $cp_sh
    fi
    for((i=0;i<${#cp_cmds[@]};i++))
    do
        tmp=${cp_cmds[$i]}
        tmp=$(echo $tmp | tr '$' ' ')
        tmp_array=($tmp)
        len=${#tmp_array[@]}
        from=$((len-2))
        to=$((len-1))
        match=$(echo "$tmp" | grep " -rf ")
        echo \[$((i+1))/${#cp_cmds[@]}\] solving copy cmd "\"$tmp\"" ...
        if [ "$match" != "" ];  then
            from_dir=${tmp_array[$from]}
            dir_name=$(echo "$from_dir" | grep -o "[^/]*$")
            cp_files[$i]="echo\$\\[$((i+1))/${#cp_cmds[@]}\\]\$coping\$\"${tmp_array[$to]}/$dir_name\" if\$[\$!\$-d\$\"${tmp_array[$to]}/$dir_name\"\$];\tthen \t${cp_cmds[$i]} fi"
        else
            cp_files[$i]="echo\$\\[$((i+1))/${#cp_cmds[@]}\\]\$coping\$\"${tmp_array[$to]}\" if\$[\$!\$-f\$\"${tmp_array[$to]}\"\$];\tthen \t${cp_cmds[$i]} fi"
        fi
        echo ${cp_files[$i]} | tr ' ' '\n' | tr '$' ' ' >> $cp_sh
    done
fi

if [ $1 -eq 3 ] || [ "$1" == "" ];   then
    if [ -f "$int_sh" ];  then
        rm $int_sh
    fi
    for((i=0;i<${#int_cmds[@]};i++))
    do
        tmp=${int_cmds[$i]}
        tmp=$(echo $tmp | tr '$' ' ')
        tmp_array=($tmp)
        len=${#tmp_array[@]}
        to=$((len-3))
        echo \[$((i+1))/${#int_cmds[@]}\] solving install_name_tool cmd "\"$tmp\"" ...
        int_files[$i]="echo\$\\[$((i+1))/${#int_cmds[@]}\\]\$changing\$name\$path\$\"${tmp_array[$to]}\" ${int_cmds[$i]}"
        echo ${int_files[$i]} | tr ' ' '\n' | tr '$' ' ' >> $int_sh
    done
fi
