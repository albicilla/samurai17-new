

R_win = 0
B_win = 0

R_num = ''
B_num = ''

results = 18

for number in range(1,results,2):

    R_num = ''
    B_num = ''

    for line in open ("generated_result"+str(number)+".out"):
        if ("time0") in line:

            skip_num = 9
            skip_num2 = 10
            idx = 0
            for i in line:
                idx += 1
                skip_num-=1
                if skip_num<0:
                    if(i==','):
                        break
                    #print (i)
                    skip_num2-=1
                    R_num = str(R_num)+i

            idx += 10
            for i in line:
                idx-=1
                if idx < 0:
                    if(i==','):
                        break
                    B_num = B_num+i


            # print (R_num)
            # print (B_num)


    R_num_1=''
    B_num_1=''
    for line in open ("generated_result"+str(number+1)+".out"):
        if ("time0") in line:

            skip_num = 9
            skip_num2 = 10
            idx = 0
            for i in line:
                idx += 1
                skip_num-=1
                if skip_num<0:
                    if(i==','):
                        break
                    #print (i)
                    skip_num2-=1
                    R_num_1 = str(R_num_1)+i

            idx += 10
            for i in line:
                idx-=1
                if idx < 0:
                    if(i==','):
                        break
                    B_num_1 = B_num_1+i

            # print (R_num_1)
            # print (B_num_1)
    total1 = float(R_num)+float(B_num_1)
    total2 = float(B_num)+float(R_num_1)
    print ("generated_result"+str(number)+".out")
    print (R_num,' ',B_num)
    print ("generated_result"+str(number+1)+".out")
    print (R_num_1,' ',B_num_1)
    print ()
    print (total1,' ',total2)

    if total1 < total2:
        R_win+=1
        print ("R")
    elif total1 > total2:
        B_win+=1
        print ("B")


print ()
print (R_win,' ',B_win)

results/=2
print (R_win*100/results,'% ',B_win*100/results,'%')
