// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;
pragma experimental ABIEncoderV2;
import "./FiscoOracleClient.sol";

contract Count is FiscoOracleClient{
    
    function uintToString(uint256 _value) internal pure returns (string memory) 
    {
        if (_value == 0) {
            return "0";
        }
        
        uint256 temp = _value;
        uint256 digits;
        while (temp != 0) {
            digits++;
            temp /= 10;
        }
        
        bytes memory buffer = new bytes(digits);
        temp = _value;
        
        for (uint256 i = digits; i > 0; i--) {
            buffer[i - 1] = bytes1(uint8(48 + (temp % 10)));
            temp /= 10;
        }
        
        return string(buffer);
    }
    
    // [新增] 用于处理包含负数的 int256 到 string 转换
    function intToString(int256 _value) internal pure returns (string memory) {
        if (_value == 0) {
            return "0";
        }
        bool isNegative = _value < 0;
        uint256 temp = isNegative ? uint256(-_value) : uint256(_value);
        uint256 digits;
        uint256 copy = temp;
        while (copy != 0) {
            digits++;
            copy /= 10;
        }
        uint256 length = isNegative ? digits + 1 : digits;
        bytes memory buffer = new bytes(length);
        for (uint256 i = length; i > (isNegative ? 1 : 0); i--) {
            buffer[i - 1] = bytes1(uint8(48 + (temp % 10)));
            temp /= 10;
        }
        if (isNegative) {
            buffer[0] = "-";
        }
        return string(buffer);
    }

    // [新增] 用于按照特定格式拼接余额字符串，如 "10|5|-50 0|30|-60"
    function buildTotalString() internal view returns (string memory) {
        bytes memory result;
        for (uint k = 0; k < Balances.length; k++) {
            if (k > 0) {
                result = abi.encodePacked(result, " "); // 接收者之间用空格隔开
            }
            result = abi.encodePacked(
                result,
                intToString(Balances[k][0]), "|",
                intToString(Balances[k][1]), "|",
                intToString(Balances[k][2])
            );
        }
        return string(result);
    }

    function joinStringArray(string[] memory arr) internal pure returns (string memory) {
        bytes memory result;
    
        for (uint i = 0; i < arr.length; i++) {
            result = abi.encodePacked(result, arr[i]);
            if (i < arr.length - 1) {
                result = abi.encodePacked(result, "|");
            }
        }
    
        return string(result);
    }
    
    string public result;
    // [修改] 将单纯的 uint256[] 改为了 int256[3][] 来分别记录3个链的变动情况
    int256[3][] private Balances;
    string[] private Receiver;
    uint256 public round = 1;
    
    // FISCO BCOS 环境下：20000 毫秒 = 20 秒
    uint256 private T = 40000; 
    
    uint256 public StartTime;

    mapping(address => uint256) public userLastAddedRound;

    // 记录接收者在当前数组中的索引
    mapping(string => uint256) public receiverIndex; 
    // 记录接收者最后一次参与的轮次
    mapping(string => uint256) public receiverLastRound; 

    // M*M的汇率矩阵
    uint256[3][3] public A = [
        [1e5, 2e5, 4e5],
        [0.5e5, 1e5, 2e5],
        [0.25e5, 0.5e5, 1e5]
    ];

    constructor(address oracleAddress) public {
        StartTime = block.timestamp; // FISCO BCOS 返回的是毫秒
        oracleCoreAddress = oracleAddress;
    }

    string public baseUrl = "json(https://5eb5-39-170-33-242.ngrok-free.app/call_sgx?input=";
    address public oracleCoreAddress;
    mapping(bytes32=>bool) private validIds;

    struct Transaction_Info {
        string Notary_id;
        uint256 Receivingchain_id;
        string Receiver_address;
        string total;
        uint256 T_select;
    }
    
    mapping(uint256=>Transaction_Info) public Txid_to_Txinfo;
    mapping(uint256 => address[]) public roundToSenders;
    mapping(uint=>string[]) public roundToReceiver;

    
    function add(uint256 i, uint256 r, string memory receiver_address, uint256 value) public returns (bytes32){
        bytes32 requestId = 0;
        // 1. 检查时间窗口 (毫秒比较)
        if (block.timestamp >= StartTime + T) {
            string memory fullUrl = string(abi.encodePacked(baseUrl));
            
            // [修改] 利用新的格式构建函数提取每个接收者变动明细
            string memory total = buildTotalString(); 
            
            fullUrl = string(abi.encodePacked(
                fullUrl,
                uintToString(Receiver.length),
                " ",
                total,
                ").max_value"
            ));
            
            requestId = oracleQuery(oracleCoreAddress, fullUrl, 0, ReturnType.STRING);

            validIds[requestId] = true; 
            roundToReceiver[round] = Receiver;
            Transaction_Info storage info = Txid_to_Txinfo[round];
            
            // 填充数据
            info.Receivingchain_id = r; 
            info.Receiver_address = joinStringArray(Receiver);
            info.total = total;
            
            // 清空数组，开启下一轮
            delete Balances; 
            delete Receiver; 

            round += 1;
            StartTime = block.timestamp;
        }

        require(userLastAddedRound[msg.sender] < round, "Already added in this round");
        require(value > 0, "Value must be greater than 0");
        require(i < 3 && r < 3, "Invalid asset index");

        // 2. 计算金额 (去精度 / 1e5)
        uint256 adjustedValue = (value * A[i][r]) / 1e5;

        // 检查该接收者在本轮是否已经是“旧数据”
        if (receiverLastRound[receiver_address] < round) {
            // === 视为新加入 ===
            receiverIndex[receiver_address] = Receiver.length;
            roundToSenders[round].push(msg.sender);
            
            Balances.push(); // Push 一个默认的[0, 0, 0]
            Receiver.push(receiver_address);
            
            receiverLastRound[receiver_address] = round;
        } 

        // === 本轮已存在或新加入后的常规赋值 ===
        uint256 idx = receiverIndex[receiver_address];
        
        // [修改] 输入链记录正资产流入，目标链记录负资产流出（支出）
        Balances[idx][i] += int256(value);
        Balances[idx][r] -= int256(adjustedValue);

        userLastAddedRound[msg.sender] = round;
        return requestId;
    }
    
    // [修改] 查看当前轮次的统计结果，返回二维数组
    function getBalances() public view returns (int256[3][] memory) {
        return Balances;
    }

    mapping(bytes32=>string) private resultMap;
    event AddressToNotary(uint256 indexed roundNumber, string receiver, string notary);
    
    function __callback(bytes32 _requestId, bytes memory _result) internal override {
        require(validIds[_requestId], "id must be not used!");
        
        result = string(_result);
        resultMap[_requestId] = result;

        if (keccak256(abi.encodePacked(result)) == keccak256(abi.encodePacked("ok"))) {
            // 回调属于 reportBan 或 reportUnlock
        } else {
            Transaction_Info storage info = Txid_to_Txinfo[round - 1];
            info.Notary_id = result; 
            info.T_select = block.timestamp; 
            
            emit AddressToNotary(round - 1, joinStringArray(roundToReceiver[round - 1]), result);
        }
        
        delete validIds[_requestId];
    }
    
    function get()  public view  returns(string memory)
    {
        return result;
    }   

    function bytesToBytes32(bytes memory source) private pure returns (bytes32 result) {
        if (source.length == 0) {
            return 0x0;
        }
        assembly {
            result := mload(add(source, 32))
        }
    }
    
    function reportBan(string memory _notaryid) external returns (bytes32) {
        string memory fullUrl = string(abi.encodePacked(baseUrl));
        fullUrl = string(abi.encodePacked(
            fullUrl, 
            _notaryid, 
            "_has_banned",
            ").max_value"
        ));
        bytes32 requestId = oracleQuery(oracleCoreAddress, fullUrl, 0, ReturnType.STRING);
        validIds[requestId] = true;
        return requestId;
    }
    
    function reportUnlock(string memory _notaryid) external returns (bytes32) {
        string memory fullUrl = string(abi.encodePacked(baseUrl));
        fullUrl = string(abi.encodePacked(
            fullUrl, 
            _notaryid, 
            "_has_Unlocked",
            ").max_value"
        ));
        bytes32 requestId = oracleQuery(oracleCoreAddress, fullUrl, 0, ReturnType.STRING);
        validIds[requestId] = true;
        return requestId;
    }
}
