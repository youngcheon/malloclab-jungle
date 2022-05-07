resultArr = [] # (trace-name, is-correct?, score)
traceArr = [
    'amptjp-bal.rep',
    'binary-bal.rep',
    'binary2-bal.rep',
    'cccp-bal.rep',
    'coalescing-bal.rep',
    'cp-decl-bal.rep',
    'expr-bal.rep',
    'random-bal.rep',
    'random2-bal.rep',
    'realloc-bal.rep',
    'realloc2-bal.rep',
    'short1.rep',
    'short2.rep',
]

import subprocess 
import os
os.system("rm -f *~ *.o mdriver")
os.system("make")
i = 0
for trace in traceArr:
    proc = subprocess.Popen(["./mdriver", "-vga", "-f", "traces/" + trace], stdout=subprocess.PIPE)
    tmpstr = proc.stdout.read()
    arr = list(map(lambda x: x.decode(), tmpstr.split()))
    is_correct = int(arr[-2].split(':')[1])
    score = int(arr[-1].split(':')[1])
    resultArr.append( (trace, bool(is_correct), score) )
    i += 1
data = subprocess.check_output("./mdriver")


print("file              vaiid? score")
print("-----------------------------")
scoreSum = 0 
for result in resultArr:
    print("{0}  {1} {2}".format(str(result[0]+' '*10)[:15], result[1], result[2]))
    scoreSum += result[2]

effectiveness = float(scoreSum) / float(100*len(traceArr)) * 100
print("------------------------------")
print("percentage: " + str(effectiveness) + " %")
print("mdriver 채점 결과 : ",data.split()[-1].decode())