#!/bin/bash


#Dnames array has the IDs
dnames_arr=()


i=0;

#Initialising counters
bytesentc=0;
byterecc=0;
filessentc=0;
filesrecc=0;
gones=0;

realcounter=0;

while read -r LINE || [[ -n $LINE ]]; do
	if echo "$LINE" | grep -q "ID: ";  then          #If line has ID: , we read the ID and add it to the array
		  dnames_arr[$i]=`echo "$LINE" | cut -c4-`
  		i=$((i + 1))
  		
  	fi

  	if echo "$LINE" | grep -q "Bytes Sent: ";  then   #If line has Bytes Sent: , we add the bytes sent to the counter
  		allbytessent=`echo "$LINE" | cut -c12-`
      bytesentc=$((allbytessent + bytesentc))
		
  			
  	fi

    if echo "$LINE" | grep -q "Bytes Received: ";  then   #If line has Bytes Received: , we add the bytes received to the counter
      allbytesrec=`echo "$LINE" | cut -c16-`
      byterecc=$((allbytesrec + byterecc))
    
        
    fi
  	
  	if echo "$LINE" | grep -q "File Sent:";  then        #If line has File Sent: , we increment the files sent counter
  		
      filessentc=$((filessentc+1))
  	fi

    if echo "$LINE" | grep -q "File Received:";  then    #If line has File Received: , we increment the files received counter
    
      filesrecc=$((filesrecc+1))
    fi
  	
  	if echo "$LINE" | grep -q "I am gone!"; then     #If line has the "I am gone!" string, we increment the counter for the clients who left
  		gones=$((gones +1))
  	fi
done


echo "A total of $i  clients entered the system"

echo ${dnames_arr[@]}

max=${dnames_arr[0]}
min=${dnames_arr[0]}

j=0;
# Loop through all elements in the array
for j in "${dnames_arr[@]}"
do
    # Update max if applicable
    if [[ "$j" -gt "$max" ]]; then
        max="$j"
    fi

    # Update min if applicable
    if [[ "$j" -lt "$min" ]]; then
        min="$j"
    fi
done

echo "Maximun ID is: $max"
echo "Mininum ID is: $min"

echo "Total bytes sent: $bytesentc" 

echo "Total bytes received: $byterecc"

echo "Total files sent: $filessentc"

echo "Total files received: $filesrecc"

echo "Client who left the system: $gones"



