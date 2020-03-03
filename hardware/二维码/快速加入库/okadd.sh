
#!/bin/bash
# curl yourNodeName yourAppKey yourDevEUI
# or add more parameters
param_file=$1
i=1
while read line;
do
   let "i=$i+1"
   arr=($line)
echo "${arr[0]}"
echo "${arr[1]}"
# curl_cmd="curl -X GET --header 'Accept: application/json' --header 'Grpc-Metadata-Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiJsb3JhLWFwcC1zZXJ2ZXIiLCJleHAiOjE1NTMzNDQ2NDIsImlzcyI6ImxvcmEtYXBwLXNlcnZlciIsIm5iZiI6MTU1MzI1ODI0Miwic3ViIjoidXNlciIsInVzZXJuYW1lIjoiYWRtaW4ifQ.0-xf9nYuESAKSmCd4dFLsWhL5KV3ak68vucaNr7Ellw' 'http://zenghi.com:8080/api/devices?applicationID=${arr[0]}'"
 curl_cmd="curl -X POST --header 'Content-Type: application/json' --header 'Accept: application/json' --header 'Grpc-Metadata-Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiJsb3JhLWFwcC1zZXJ2ZXIiLCJleHAiOjE1NTMzNDQ2NDIsImlzcyI6ImxvcmEtYXBwLXNlcnZlciIsIm5iZiI6MTU1MzI1ODI0Miwic3ViIjoidXNlciIsInVzZXJuYW1lIjoiYWRtaW4ifQ.0-xf9nYuESAKSmCd4dFLsWhL5KV3ak68vucaNr7Ellw' -d '{ \"deviceActivation\": { \"aFCntDown\": 0, \"appSKey\": \"1808031628AED2A6ABF7158809CF4F3C\",\"devAddr\": \"${arr[0]}\", \"devEUI\": \"02180803${arr[0]}\",\"fCntUp\": 0,\"fNwkSIntKey\": \"1808031628AED2A6ABF7158809CF4F3C\",\"nwkSEncKey\": \"1808031628AED2A6ABF7158809CF4F3C\",\"sNwkSIntKey\": \"1808031628AED2A6ABF7158809CF4F3C\", \"skipFCntCheck\": true }}' 'http://zenghi.com:8080/api/devices/02180803${arr[0]}/activate'"
 eval "$curl_cmd"
 done < $param_file
 echo “e123nd”
