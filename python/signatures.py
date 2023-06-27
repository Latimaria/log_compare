#!/usr/bin/python3

import re
import json

function_signatures = {}
function_classes = {}
func = 'chooseRandom(int,java.lang.String,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['chooseRandom'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'addIfIsGoodTarget(org.apache.hadoop.hdfs.server.blockmanagement.DatanodeStorageInfo,java.util.Set,long,int,boolean,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['addIfIsGoodTarget'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'isGoodTarget(org.apache.hadoop.hdfs.server.blockmanagement.DatanodeStorageInfo,long,int,boolean,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['isGoodTarget'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'chooseLocalStorage(org.apache.hadoop.net.Node,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType,boolean)'
function_signatures['chooseLocalStorage'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'

func = 'rollBlocksScheduled(long)'
function_signatures['rollBlocksScheduled'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.DatanodeDescriptor'
func = '<init>(java.lang.String,java.lang.String,java.lang.String,int,int,int,int,long,long,long,long,long,long,long,int,java.lang.String,org.apache.hadoop.hdfs.protocol.DatanodeInfo$AdminStates)'
function_signatures['DatanodeInfo'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.DatanodeInfo'
func = 'setRemaining(long)'
function_signatures['setRemaining'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.DatanodeInfo'

func = 'chooseLocalStorage(org.apache.hadoop.net.Node,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType,boolean)'
function_signatures['chooseLocalStorage'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'chooseTarget(int,org.apache.hadoop.net.Node,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['chooseTarget'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'chooseTargets(org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicy,java.util.Set)'
function_signatures['chooseTargets'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockManager$ReplicationWork'

func = 'chooseRemoteRack(int,org.apache.hadoop.hdfs.server.blockmanagement.DatanodeDescriptor,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['chooseRemoteRack'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'

func = 'set(long,long,long)'
function_signatures['set'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.Block'
func = 'setNumBytes(long)'
function_signatures['setNumBytes'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.Block'

func = 'updateBlocks(org.apache.hadoop.hdfs.server.namenode.FSDirectory,oorg.apache.hadoop.hdfs.server.namenode.FSEditLogOp$BlockListUpdatingOp,org.apache.hadoop.hdfs.server.namenode.INodeFile)'
function_signatures['updateBlocks'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.namenode.FSEditLogLoader'
func = 'addNewBlock(org.apache.hadoop.hdfs.server.namenode.FSDirectory,org.apache.hadoop.hdfs.server.namenode.FSEditLogOp$org.apache.hadoop.hdfs.server.namenode.INodeFile)'
function_signatures['addNewBlock'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.namenode.FSEditLogLoader'

with open('function_signatures.json', 'w') as f:
    json.dump(function_signatures, f)

with open('function_classes.json', 'w') as c:
    json.dump(function_classes, c)
