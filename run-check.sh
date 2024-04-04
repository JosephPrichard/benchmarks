files=$(find ./output -type f -name 'out-*.txt')

arr=()
for file in $files; do
    echo "$file"
    arr+=( "$file" )
done

python ./check-results.py "${arr[@]}"