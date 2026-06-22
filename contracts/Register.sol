// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;
pragma experimental ABIEncoderV2;
import "./FiscoOracleClient.sol";
//继承FiscoOracleClient合约
contract Register is FiscoOracleClient {
    //添加了密文数组，希望可以传入到sgx中，sgx存储结果到文本文件，后续在sgx内将文本转为json文件
    struct CipherData {
        string ciphertext;
        string iv;
        string tag;
        string aad;
        string pub;
    }
    
    struct Registration_Info {
        uint256 Notary_id;
        string Notary_ip;
        string ciphertext;
        string iv;
        string tag;
        string aad;
        string pub;
    }
    struct Trust{
        uint256 value;
        uint8 state; //0:unlocked  1:locked   2:lockedforever
    }
    Trust[] public TrustbyId;
    //notary
    event notary(uint256 indexed id, string ip);
    //密文数组
    Registration_Info[] public registerlist;
    
    
    
    // 输出所有 CipherData[]
    function getAllCipherData() public view returns (CipherData[] memory) {
        return cipherList;
    }
    //指定处理的oracle
    address public oracleCoreAddress;

    // Multiply the result by 1000000000000000000 to remove decimals
    uint256 private timesAmount  = 10**18;
    
    mapping(bytes32=>string) private resultMap;

    mapping(bytes32=>bool) private validIds;
    string public result;
    //string private url = "json(https://6f66e3eabba3.ngrok-free.app/call_sgx?input=hello).max_value";
        // 基础URL，使用timesAmount作为参数
    string private baseUrl = "json(https://336c7df1d037.ngrok-free.app/call_sgx?input=";
    address public owner;
    uint256 private _id;
    string private _ip;


    
    constructor(address oracleAddress) public {
        oracleCoreAddress = oracleAddress;
        owner = msg.sender;

    }

    function uintToString(uint256 _value) internal pure returns (string memory) {
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

    
        //密文数组
    CipherData[] public cipherList;
    
    function storeCipherData(//调用虚拟机存储
        string memory _ip,
        string memory _ciphertext,
        string memory _iv,
        string memory _tag,
        string memory _aad,
        string memory _pub
    ) public returns (bytes32) {
        emit notary(_id, _ip);
        registerlist.push(Registration_Info(_id,_ip,_ciphertext, _iv, _tag, _aad, _pub));
        TrustbyId.push(Trust(20,0));
        string memory fullUrl = string(abi.encodePacked(baseUrl));
        fullUrl = string(abi.encodePacked(
                fullUrl,
                _ciphertext,
                " ",
                _iv,
                " ",
                _tag,
                " ",
                _aad,
                " ",
                _pub,
                ").max_value"
            ));
        //bytes32 requestId = oracleQuery(oracleCoreAddress, fullUrl, timesAmount, returnType);
        bytes32 requestId = oracleQuery(oracleCoreAddress, fullUrl, 0, ReturnType.STRING);
        validIds[requestId] = true; 
        _id+=1;
        return requestId;
    }



    /**
     * Receive the response in the form of int256
     */
    function __callback(bytes32 _requestId, bytes memory _result) internal override
    {
        require(validIds[_requestId], "id must be not used!") ;
        result =  string(_result);
        //result =   int256(bytesToBytes32(_result));//if result==-1则push进Notary结构体中
        resultMap[_requestId]= result;
        delete validIds[_requestId];

    }

      function get()  public view  returns(string memory){
          
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
    
    function getTrustValuebyId(uint256 _id) public view returns (uint256){
        return TrustbyId[_id].value;
    }
    
}