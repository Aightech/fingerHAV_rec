#!/bin/bash
# This script is used to extract the data from the database and store it in a file

# the list of texture to extract
#textures=( "55t" "38t" "54t" "18t" "108t" "23t" "1t" "57t" "83t" "14t" )
textures=( "14t" )

#id of the texture
#ids=("1" "2" "3" "4" "5")
ids=("7")

for i in "${!textures[@]}"; do
    echo "Extracting ${textures[$i]}"
    mkdir -p ${textures[$i]}
    cd ${textures[$i]}
    for j in "${!ids[@]}"; do
	echo "Extracting ${ids[$j]}"
	mkdir -p ${textures[$i]}_${ids[$j]}
	cd ${textures[$i]}_${ids[$j]}
	python3 ../../psql2mat.py forces ${textures[$i]}_${ids[$j]}
	python3 ../../psql2mat.py positions ${textures[$i]}_${ids[$j]}
	python3 ../../psql2mat.py Kistler ${textures[$i]}_${ids[$j]}
	cd ..
    done
    cd ..
done
