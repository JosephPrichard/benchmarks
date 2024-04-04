files=$(find ./output -type f -name 'bench-out-*.csv')

arr=()
for file in $files; do
    echo "$file"
    arr+=( "$file" )
done

python ./graph-results.py "${arr[@]}"