id=$1
port=$2
echo $id
echo $port
cd ~/hadoop/
reportdir=~/hadoop/hadoop-hdfs-project/hadoop-hdfs/target/surefire-reports/
logfile=$reportdir/org.apache.hadoop.hdfs.server.blockmanagement.TestPendingReplication-output.txt
instrumentation_plan_dir=~/log/log_compare/python/plans
rm $reportdir/*

file=$instrumentation_plan_dir/current_b$id.btm
echo $file

mvn test -Dtest=TestPendingReplication -DargLine="-javaagent:~/byteman-download-4.0.21/lib/byteman.jar=listener:true,boot:~/byteman-download-4.0.21/lib/byteman.jar,script:$file,port:$port" > /dev/null &
mvn_pid=$!

while ! test -f "$logfile"
do
  sleep 1
  echo "Still waiting"
done
sleep 10
bmsubmit.sh -u
testpid=$(jps | grep surefire | awk '{print $1}')
bminstall.sh $testpid
bmsubmit.sh $file
bmsubmit.sh
sleep 10
grep 'BM' $logfile > ~/log/log_compare/python/logs/current_b$id.log
awk '/Start Stack Trace/,/End Stack Trace/' $logfile >> ~/log/log_compare/python/logs/current_b$id.log
echo $logfile

cd -
echo current_b$id.log


