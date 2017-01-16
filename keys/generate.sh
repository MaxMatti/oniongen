/bin/bash

for i in `seq 0 1000000`
do
  openssl genrsa -out $i.csr 1024 1&>2 /dev/null
  openssl rsa -in $i.csr -out $i.pub -outform DER -pubout 1&>2  /dev/null
done
