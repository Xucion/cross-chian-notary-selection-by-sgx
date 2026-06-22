// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;
pragma experimental ABIEncoderV2;
import "./FiscoOracleClient.sol";
// ---------- interface for register ----------
interface IRegister {
    function getTrustValuebyId(uint256 _id) external view returns (uint256);
}

// ---------- interface for newAPISampleOracle ----------
interface INewAPISampleOracle {
    struct Transaction_Info {
        string Notary_id;
        uint256 Receivingchain_id;
        string Receiver_address;
        string total;
        uint256 T_select;
    }

    function Txid_to_Txinfo(uint256 round) external view returns (
        string memory Notary_id,
        uint256 Receivingchain_id,
        string memory Receiver_address,
        string memory total,
        uint256 T_select
    );
    function reportBan(string memory _notaryid) external returns (bytes32);
    function reportUnlock(string memory _notaryid) external returns (bytes32);

}

// ---------- 主合约 ----------
contract TrustManagement {
    IRegister public register;
    INewAPISampleOracle public oracle;

    constructor(address registerAddr, address oracleAddr) public {
        register = IRegister(registerAddr);
        oracle = INewAPISampleOracle(oracleAddr);
    }

    function readTrustValue(uint256 _id) public view returns (uint256) {
        return register.getTrustValuebyId(_id);
    }

    function readTxInfo(uint256 round) public view returns (
        string memory Notary_id,
        uint256 Receivingchain_id,
        string memory Receiver_address,
        string memory total,
        uint256 T_select
    ) {
        return oracle.Txid_to_Txinfo(round);
    }
    
    // 被禁止发起投票的用户
    mapping(address => bool) public banned;
    
    
    struct VoteInfo {
        address creator; // 发起人
        string proposal; // 提案内容
        bool voting; // 是否进行中
        uint agreeCount; // 赞成票数
        uint disagreeCount;
        bool passed; // 是否通过
        mapping(address => bool) voted; // 是否已投票
    }
    
    
    // round => seq => VoteInfo
    mapping(uint8 => mapping(uint8 => VoteInfo)) private votes;

    event VoteStarted(uint8 indexed round, uint8 indexed seq, address indexed creator, string proposal);
    event VoteCast(uint8 indexed round, uint8 indexed seq, address indexed voter, string notaryId, bool agree);
    event VoteFinalized(uint8 indexed round, uint8 indexed seq, bool agreeSideWon);

    function getVote(uint8 round, uint8 seq) public view returns (
        address creator,
        string memory proposal,
        bool voting,
        uint agreeCount,
        uint disagreeCount,
        bool passed
    ) {
        VoteInfo storage v = votes[round][seq];
        return (v.creator, v.proposal, v.voting, v.agreeCount, v.disagreeCount, v.passed);
    }
    
    
    // 发起投票（任何人都可以，未被封禁）
    function startVote(
        uint8 round,
        uint8 seq,
        string memory _proposal
        ) public {
        require(!banned[msg.sender], "You are banned");
        require(!votes[round][seq].voting, "Vote already exists");
        
        
        VoteInfo storage v = votes[round][seq];
        
        
        v.creator = msg.sender;
        v.proposal = _proposal;
        v.voting = true;
        v.agreeCount = 0;
        v.disagreeCount = 0;
        v.passed = false;

        emit VoteStarted(round, seq, msg.sender, _proposal);
    }
    
    
    // 投票（B/C/D等）
    function vote(
        uint8 round,
        uint8 seq,
        string memory _notaryid,
        bool agree
        ) public {
        VoteInfo storage v = votes[round][seq];
        
        
        require(v.voting, "No active vote");
        require(!v.voted[msg.sender], "Already voted");
        
        
        v.voted[msg.sender] = true;
        emit VoteCast(round, seq, msg.sender, _notaryid, agree);
        
        
        if (agree) {
            v.agreeCount = v.agreeCount + 1;
            // 超过2票自动通过
            if (v.agreeCount > 2) {
                v.passed = true;
                v.voting = false;
                oracle.reportUnlock(_notaryid);
                emit VoteFinalized(round, seq, true);
            }
        }
        else {
            v.disagreeCount = v.disagreeCount + 1;
            if (v.disagreeCount > 2)  {
                v.passed = true;
                v.voting = false;
                oracle.reportBan(_notaryid);
                emit VoteFinalized(round, seq, false);
            }
        }
    
        }
}
