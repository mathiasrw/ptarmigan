#!/bin/sh
# amount appear in commit_tx
PAY_BEGIN=3333
PAY_END=4444
AMOUNT=200000000

echo "-------------------------------"
echo "PAY large: 3333-->4444"
echo "-------------------------------"

./example_st4pay_r.sh $PAY_BEGIN $PAY_END $AMOUNT
