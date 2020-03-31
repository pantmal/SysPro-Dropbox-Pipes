#!/bin/bash    


set -e


if [ "$#" -lt 4 ]; then
	echo "We need more parameters..."
	exit 1
fi

if [ "$2" -lt 0 ]; then
	echo "Please, no negative numbers"
	exit 1
fi

if [ "$3" -lt 0 ]; then
	echo "Please, no negative numbers"
	exit 1
fi

if [ "$4" -lt 0 ]; then
	echo "Please, no negative numbers"
	exit 1
fi

num_of_files=$2
num_of_dirs=$3
levels=$4


mkdir -p "$1"


#Using an array for the dir names
dnames_arr=()


#Creating the dir names using urandom
for ((i =1;  i  <= num_of_dirs;i ++)) do
    dnames_arr[$i]=`head /dev/urandom | tr -dc A-Za-z0-9 | head -c $((1+RANDOM%8)) ; echo ''` 
done
declare -p dnames_arr



levelc=0;
dirc=1;


#Getting home directory
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

#FILE_NAME variable has the path of the input dir
FILE_NAME=$SCRIPTPATH/$1



#Creating the directories here
while [ $dirc -le $num_of_dirs ]
	do
	mkdir "${dnames_arr[$dirc]}" #Create the dir
	if [ $levelc -eq 0 ]; then #Slightly different work if the level counter is more than 0

		mv "${dnames_arr[$dirc]}" $FILE_NAME     #Move it to the input dir
		TEMPATH=$FILE_NAME/${dnames_arr[$dirc]}  #Next path will the one we already were + 1 level
		
		dirc=$((dirc + 1))
		levelc=$((levelc + 1))
		if [ $levelc -eq $levels ]  #If level counter reached the value of the levels parameter then
		then
			levelc=0
		fi
		continue;
	fi

	if [ $levelc -gt 0 ]; then

		mv "${dnames_arr[$dirc]}" $TEMPATH    #Move it to the desired path
		TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
		dirc=$((dirc + 1))
		levelc=$((levelc + 1))     #And reset the level counter as above
		if [ $levelc -eq $levels ]
		then
			levelc=0
		fi
		continue;
	fi
	done
echo

fc=1;
dirc=1;
levelc=0;

#Also using an array for the file names
fnames_arr=()

#Create the file names, same as before
for ((i =1;  i  <= num_of_files;i ++)) do
    fnames_arr[$i]=`head /dev/urandom | tr -dc A-Za-z0-9 | head -c $((1+RANDOM%8)) ; echo ''` 
done
declare -p fnames_arr

numone=1;

offset=1;
TEMPATH=$FILE_NAME
while [ $fc -le $num_of_files ]               #Creating the files and moving them at an appropiate directory
	do

	if [ $numone -eq $num_of_files ]; then   #Corner case if we have only one file
		touch "${fnames_arr[$fc]}"
		chmod 777 "${fnames_arr[$fc]}"
		mv "${fnames_arr[$fc]}" $FILE_NAME	

	fi
	

	if [ $fc -lt $num_of_files ]; then        #The first time, we enter the loop, we place the file at the input dir
		
		touch "${fnames_arr[$fc]}"
		chmod 777 "${fnames_arr[$fc]}"
		mv "${fnames_arr[$fc]}" $FILE_NAME	

	fi


	fc=$((fc + 1)) 
	if [ $fc -gt $num_of_files ]; then     #If counter exceeded the number of files, exit the loop
				

				
				continue;
	fi
	
	TEMPATH=$TEMPATH/${dnames_arr[$dirc]}    #Proceed to the next subdir
	while [ $dirc -le $num_of_dirs ]
		
		do
		if [ $levelc -eq 0 ]; then
			touch "${fnames_arr[$fc]}"
			chmod 777 "${fnames_arr[$fc]}"
			mv "${fnames_arr[$fc]}" $TEMPATH        #Create the file and place it


			dirc=$((dirc + 1))                      #Increment the directory counter
			

			fc=$((fc + 1))                          #Increment the file counter
			if [ $fc -eq $num_of_files ]; then      #If we reached the limit, create the file, place it and exit the loop

				touch "${fnames_arr[$fc]}"
				chmod 777 "${fnames_arr[$fc]}"
				TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
				mv "${fnames_arr[$fc]}" $TEMPATH
				break;
			fi
			levelc=$((levelc + 1))                #Otherwise move to the path of the following level, or reset it if we reached the limit of the levels
			if [ $levelc -eq $levels ]
			then
				TEMPATH=$FILE_NAME
				levelc=0
			else
				TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
			fi
			continue;
		fi

		if [ $levelc -gt 0 ]; then                 #Slighlty different work if level counter is greater than 0. 
			if [ $fc -lt $num_of_files ]; then     #Creating and placing the file at the desired spot
		
				touch "${fnames_arr[$fc]}"
				chmod 777 "${fnames_arr[$fc]}"
				mv "${fnames_arr[$fc]}" $TEMPATH

			fi
			

			dirc=$((dirc + 1))			
			fc=$((fc + 1))
			
			levelc=$((levelc + 1))
			if [ $levelc -eq $levels ]                        #Offset variable is used here to get the path of the next subdirs we want
			then											  #Example: If we have levels=2 and we reached 2 we will be doing work for the dir3 folder
				offset=$((offset+levels))					  #Since we were at dir1, we will get path of dir3 because 1+2=3
				TEMPATH=$FILE_NAME/${dnames_arr[$offset]}
				
				
				levelc=0
			else
				TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
			fi
			if [ $fc -eq $num_of_files ]; then              #Break if we reached the file limit
				touch "${fnames_arr[$fc]}"
			
				chmod 777 "${fnames_arr[$fc]}"
				mv "${fnames_arr[$fc]}" $TEMPATH
				break;
			fi
			continue;

		fi

	done
	levelc=0;                       #Reseting the variables here
	dirc=1;
	offset=1;
	TEMPATH=$FILE_NAME

	done
echo

thousand=1000;

for file in $FILE_NAME/*             #Now we place the strings at every file, starting from the input dir
do
  
  if [ -f "$file" ]; then
  	randis=`shuf -i 1000-128000 -n 1`
  	tempstring=`base64 /dev/urandom | tr -dc A-Za-z0-9 | head -c $randis ; echo '' ` #Create a random string and place it inside the file
  	echo "$tempstring" >> "$file"
  fi
done

TEMPATH=$FILE_NAME
offset=1;
TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
while [ $dirc -le $num_of_dirs ]         #Now let's proceed to the subdirs

	do

		if [ $levelc -eq 0 ]; then
		
		for file in $TEMPATH/*         #For every file in the path we're in
		do
  			if [ -f "$file" ]; then
  				randis=`shuf -i 1000-128000 -n 1`
  				tempstring=`base64 /dev/urandom | tr -dc A-Za-z0-9 | head -c $randis ; echo '' `   #Create a random string and place it inside the file
  				echo "$tempstring" >> "$file"
  			fi
  		done
  		dirc=$((dirc + 1))	
  		levelc=$((levelc + 1))
		if [ $levelc -eq $levels ]              #Now move to the path of the following level, or reset it if we reached the limit of the levels
		then
			TEMPATH=$FILE_NAME
			levelc=0
		else
			TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
		fi
		
		continue;
		fi

		if [ $levelc -gt 0 ]; then
		
		for file in $TEMPATH/*
		do
  			if [ -f "$file" ]; then
  				randis=`shuf -i 1000-128000 -n 1`
  				tempstring=`base64 /dev/urandom | tr -dc A-Za-z0-9 | head -c $randis ; echo '' ` 
  				echo "$tempstring" >> "$file"
  			fi
  		done
  		dirc=$((dirc + 1))	
  		levelc=$((levelc + 1))
		if [ $levelc -eq $levels ]                #Using the offset variable, same as before
		then
			offset=$((offset+levels))
			TEMPATH=$FILE_NAME/${dnames_arr[$offset]}
			levelc=0
		else
			TEMPATH=$TEMPATH/${dnames_arr[$dirc]}
		fi
		
		continue;
		fi


	done

echo