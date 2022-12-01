#!/usr/bin/awk -f
{
    if (NF==0) {
        if (cal > max1) {
            max3=max2; max2=max1; max1=cal;
        }else if (cal > max2) {
            max3=max2; max2=cal;
        } else if (cal > max3) {
            max3 = cal;
        }
        cal = 0;
    } else {
        cal+=$1
    }
}END{
    if (cal > max1) {
        max3=max2; max2=max1; max1=cal;
    }else if (cal > max2) {
        max3=max2; max2=cal;
    } else if (cal > max3) {
        max3 = cal;
    }
     
     print max1,(max1+max2+max3);
}
