
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
 curl_cmd="curl -X POST --header 'Content-Type: application/json' --header 'Accept: application/json' --header 'Grpc-Metadata-Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiJsb3JhLWFwcC1zZXJ2ZXIiLCJleHAiOjE1NTMzNDQ2NDIsImlzcyI6ImxvcmEtYXBwLXNlcnZlciIsIm5iZiI6MTU1MzI1ODI0Miwic3ViIjoidXNlciIsInVzZXJuYW1lIjoiYWRtaW4ifQ.0-xf9nYuESAKSmCd4dFLsWhL5KV3ak68vucaNr7Ellw' -d '{ \"device\": { \"applicationID\": \"5\", \"description\": \"CD\",\"devEUI\": \"02180803${arr[0]}\", \"deviceProfileID\": \"7aca1707-eb81-4b06-9096-85f64860a03e\",\"name\": \"02180803${arr[0]}\",\"referenceAltitude\": 0, \"skipFCntCheck\": true }}' 'http://zenghi.com:8080/api/devices'"
 eval "$curl_cmd"
 done < $param_file
 echo “e123nd”
