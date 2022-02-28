#ifndef BPV6_H
#define BPV6_H

#include <cstdint>
#include <cstddef>
#include "Cbhe.h"
#include "TimestampUtil.h"
#include "codec/PrimaryBlock.h"
#include "EnumAsFlagsMacro.h"
#include <array>
#include "FragmentSet.h"

// (1-byte version) + (1-byte sdnv block length) + (1-byte sdnv zero dictionary length) + (up to 14 10-byte sdnvs) + (32 bytes hardware accelerated SDNV overflow instructions) 
#define CBHE_BPV6_MINIMUM_SAFE_PRIMARY_HEADER_ENCODE_SIZE (1 + 1 + 1 + (14*10) + 32)

// (1-byte block type) + (2 10-byte sdnvs) + (32 bytes hardware accelerated SDNV overflow instructions) 
#define BPV6_MINIMUM_SAFE_CANONICAL_HEADER_ENCODE_SIZE (1 + (2*10) + 32)

// (1-byte block type) + (2 10-byte sdnvs) + primary
#define CBHE_BPV6_MINIMUM_SAFE_PRIMARY_PLUS_CANONICAL_HEADER_ENCODE_SIZE (1 + (2*10) + CBHE_BPV6_MINIMUM_SAFE_PRIMARY_HEADER_ENCODE_SIZE)

#define BPV6_CCSDS_VERSION        (6)
#define BPV6_5050_TIME_OFFSET     (946684800)

#define bpv6_unix_to_5050(time)          (time - BPV6_5050_TIME_OFFSET)
#define bpv6_5050_to_unix(time)          (time + BPV6_5050_TIME_OFFSET)

enum class BPV6_PRIORITY : uint64_t {
    BULK = 0,
    NORMAL = 1,
    EXPEDITED = 2
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_PRIORITY);

enum class BPV6_BUNDLEFLAG : uint64_t {
    NO_FLAGS_SET                          = 0,
    ISFRAGMENT                            = 1 << 0,
    ADMINRECORD                           = 1 << 1,
    NOFRAGMENT                            = 1 << 2,
    CUSTODY_REQUESTED                     = 1 << 3,
    SINGLETON                             = 1 << 4,
    USER_APP_ACK_REQUESTED                = 1 << 5,
    PRIORITY_BULK                         = (static_cast<uint64_t>(BPV6_PRIORITY::BULK)) << 7,
    PRIORITY_NORMAL                       = (static_cast<uint64_t>(BPV6_PRIORITY::NORMAL)) << 7,
    PRIORITY_EXPEDITED                    = (static_cast<uint64_t>(BPV6_PRIORITY::EXPEDITED)) << 7,
    PRIORITY_BIT_MASK                     = 3 << 7,
    RECEPTION_STATUS_REPORTS_REQUESTED    = 1 << 14,
    CUSTODY_STATUS_REPORTS_REQUESTED      = 1 << 15,
    FORWARDING_STATUS_REPORTS_REQUESTED   = 1 << 16,
    DELIVERY_STATUS_REPORTS_REQUESTED     = 1 << 17,
    DELETION_STATUS_REPORTS_REQUESTED     = 1 << 18
};
MAKE_ENUM_SUPPORT_FLAG_OPERATORS(BPV6_BUNDLEFLAG);
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_BUNDLEFLAG);

//#define bpv6_bundle_set_priority(flags)  ((uint32_t)((flags & 0x000003) << 7))
//#define bpv6_bundle_get_priority(flags)  ((BPV6_PRIORITY)((flags & 0x000180) >> 7))
BOOST_FORCEINLINE BPV6_PRIORITY GetPriorityFromFlags(BPV6_BUNDLEFLAG flags) {
    return static_cast<BPV6_PRIORITY>(((static_cast<std::underlying_type<BPV6_BUNDLEFLAG>::type>(flags)) >> 7) & 3);
}


/**
 * Structure that contains information necessary for an RFC5050-compatible primary block
 */
struct Bpv6CbhePrimaryBlock : public PrimaryBlock {
    BPV6_BUNDLEFLAG m_bundleProcessingControlFlags;
    uint64_t m_blockLength;
    cbhe_eid_t m_destinationEid;
    cbhe_eid_t m_sourceNodeId;
    cbhe_eid_t m_reportToEid;
    cbhe_eid_t m_custodianEid;
    TimestampUtil::bpv6_creation_timestamp_t m_creationTimestamp;
    uint64_t m_lifetimeSeconds;
    uint64_t m_fragmentOffset;
    uint64_t m_totalApplicationDataUnitLength;

    

    Bpv6CbhePrimaryBlock(); //a default constructor: X()
    ~Bpv6CbhePrimaryBlock(); //a destructor: ~X()
    Bpv6CbhePrimaryBlock(const Bpv6CbhePrimaryBlock& o); //a copy constructor: X(const X&)
    Bpv6CbhePrimaryBlock(Bpv6CbhePrimaryBlock&& o); //a move constructor: X(X&&)
    Bpv6CbhePrimaryBlock& operator=(const Bpv6CbhePrimaryBlock& o); //a copy assignment: operator=(const X&)
    Bpv6CbhePrimaryBlock& operator=(Bpv6CbhePrimaryBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6CbhePrimaryBlock & o) const; //operator ==
    bool operator!=(const Bpv6CbhePrimaryBlock & o) const; //operator !=
    void SetZero();
    uint64_t SerializeBpv6(uint8_t * serialization) const;
    uint64_t GetSerializationSize() const;
    bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    
    /**
     * Dumps a primary block to stdout in a human-readable way
     *
     * @param primary Primary block to print
     */
    void bpv6_primary_block_print() const;

    
    


    virtual bool HasCustodyFlagSet() const;
    virtual bool HasFragmentationFlagSet() const;
    virtual cbhe_bundle_uuid_t GetCbheBundleUuidFromPrimary() const;
    virtual cbhe_bundle_uuid_nofragment_t GetCbheBundleUuidNoFragmentFromPrimary() const;
    virtual cbhe_eid_t GetFinalDestinationEid() const;
    virtual uint8_t GetPriority() const;
    virtual uint64_t GetExpirationSeconds() const;
    virtual uint64_t GetSequenceForSecondsScale() const;
    virtual uint64_t GetExpirationMilliseconds() const;
    virtual uint64_t GetSequenceForMillisecondsScale() const;
};

// https://www.iana.org/assignments/bundle/bundle.xhtml#block-types
enum class BPV6_BLOCK_TYPE_CODE : uint8_t {
    PRIMARY_IMPLICIT_ZERO             = 0,
    PAYLOAD                           = 1,
    BUNDLE_AUTHENTICATION             = 2,
    PAYLOAD_INTEGRITY                 = 3,
    PAYLOAD_CONFIDENTIALITY           = 4,
    PREVIOUS_HOP_INSERTION            = 5,
    UNUSED_6                          = 6,
    UNUSED_7                          = 7,
    METADATA_EXTENSION                = 8,
    EXTENSION_SECURITY                = 9,
    CUSTODY_TRANSFER_ENHANCEMENT      = 10,
    UNUSED_11                         = 11,
    UNUSED_12                         = 12,
    BPLIB_BIB                         = 13,
    BUNDLE_AGE                        = 20,
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_BLOCK_TYPE_CODE);

enum class BPV6_BLOCKFLAG : uint64_t {
    NO_FLAGS_SET                                        = 0,
    MUST_BE_REPLICATED_IN_EVERY_FRAGMENT                = 1 << 0,
    STATUS_REPORT_REQUESTED_IF_BLOCK_CANT_BE_PROCESSED  = 1 << 1,
    DELETE_BUNDLE_IF_BLOCK_CANT_BE_PROCESSED            = 1 << 2,
    IS_LAST_BLOCK                                       = 1 << 3,
    DISCARD_BLOCK_IF_IT_CANT_BE_PROCESSED               = 1 << 4,
    BLOCK_WAS_FORWARDED_WITHOUT_BEING_PROCESSED         = 1 << 5,
    BLOCK_CONTAINS_AN_EID_REFERENCE_FIELD               = 1 << 6,
};
MAKE_ENUM_SUPPORT_FLAG_OPERATORS(BPV6_BLOCKFLAG);
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_BLOCKFLAG);

/**
 * Structure that contains information necessary for a 5050-compatible canonical block
 */
struct Bpv6CanonicalBlock {
    BPV6_BLOCKFLAG m_blockProcessingControlFlags;
    uint64_t m_blockTypeSpecificDataLength;
    uint8_t * m_blockTypeSpecificDataPtr; //if NULL, data won't be copied (just allocated)
    BPV6_BLOCK_TYPE_CODE m_blockTypeCode; //should be at beginning but here do to better packing

    Bpv6CanonicalBlock(); //a default constructor: X()
    virtual ~Bpv6CanonicalBlock(); //a destructor: ~X()
    Bpv6CanonicalBlock(const Bpv6CanonicalBlock& o); //a copy constructor: X(const X&)
    Bpv6CanonicalBlock(Bpv6CanonicalBlock&& o); //a move constructor: X(X&&)
    Bpv6CanonicalBlock& operator=(const Bpv6CanonicalBlock& o); //a copy assignment: operator=(const X&)
    Bpv6CanonicalBlock& operator=(Bpv6CanonicalBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6CanonicalBlock & o) const; //operator ==
    bool operator!=(const Bpv6CanonicalBlock & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_blockTypeSpecificDataPtr to serialized location
    uint64_t GetSerializationSize() const;
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    static bool DeserializeBpv6(std::unique_ptr<Bpv6CanonicalBlock> & canonicalPtr, const uint8_t * serialization,
        uint64_t & numBytesTakenToDecode, uint64_t bufferSize, const bool isAdminRecord);
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();
    //virtual bool Virtual_DeserializeExtensionBlockDataBpv7();
    /**
     * Dumps a canonical block to stdout in a human-readable fashion
     *
     * @param block Canonical block which should be displayed
     */
    void bpv6_canonical_block_print() const;

    /**
     * Print just the block flags for a generic canonical block
     *
     * @param block The canonical block with flags to be displayed
     */
    void bpv6_block_flags_print() const;

};


struct Bpv6CustodyTransferEnhancementBlock : public Bpv6CanonicalBlock {
    
public:
    static constexpr unsigned int CBHE_MAX_SERIALIZATION_SIZE =
        1 + //block type
        10 + //block flags sdnv +
        1 + //block length (1-byte-min-sized-sdnv) +
        10 + //custody id sdnv +
        45; //length of "ipn:18446744073709551615.18446744073709551615" (note 45 > 32 so sdnv hardware acceleration overwrite is satisfied)

    uint64_t m_custodyId;
    std::string m_ctebCreatorCustodianEidString;

public:
    Bpv6CustodyTransferEnhancementBlock(); //a default constructor: X()
    virtual ~Bpv6CustodyTransferEnhancementBlock(); //a destructor: ~X()
    Bpv6CustodyTransferEnhancementBlock(const Bpv6CustodyTransferEnhancementBlock& o); //a copy constructor: X(const X&)
    Bpv6CustodyTransferEnhancementBlock(Bpv6CustodyTransferEnhancementBlock&& o); //a move constructor: X(X&&)
    Bpv6CustodyTransferEnhancementBlock& operator=(const Bpv6CustodyTransferEnhancementBlock& o); //a copy assignment: operator=(const X&)
    Bpv6CustodyTransferEnhancementBlock& operator=(Bpv6CustodyTransferEnhancementBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6CustodyTransferEnhancementBlock & o) const; //operator ==
    bool operator!=(const Bpv6CustodyTransferEnhancementBlock & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_dataPtr to serialized location
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();
};

//https://datatracker.ietf.org/doc/html/rfc6259
struct Bpv6PreviousHopInsertionCanonicalBlock : public Bpv6CanonicalBlock {
    static constexpr uint64_t largestSerializedDataOnlySize =
        4 + //ipn\0
        20 + // 18446744073709551615
        1 + // :
        20 + // 18446744073709551615
        1; // \0

    Bpv6PreviousHopInsertionCanonicalBlock(); //a default constructor: X()
    virtual ~Bpv6PreviousHopInsertionCanonicalBlock(); //a destructor: ~X()
    Bpv6PreviousHopInsertionCanonicalBlock(const Bpv6PreviousHopInsertionCanonicalBlock& o); //a copy constructor: X(const X&)
    Bpv6PreviousHopInsertionCanonicalBlock(Bpv6PreviousHopInsertionCanonicalBlock&& o); //a move constructor: X(X&&)
    Bpv6PreviousHopInsertionCanonicalBlock& operator=(const Bpv6PreviousHopInsertionCanonicalBlock& o); //a copy assignment: operator=(const X&)
    Bpv6PreviousHopInsertionCanonicalBlock& operator=(Bpv6PreviousHopInsertionCanonicalBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6PreviousHopInsertionCanonicalBlock & o) const; //operator ==
    bool operator!=(const Bpv6PreviousHopInsertionCanonicalBlock & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_dataPtr to serialized location
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();

    cbhe_eid_t m_previousNode;
};

//https://datatracker.ietf.org/doc/html/draft-irtf-dtnrg-bundle-age-block-01
struct Bpv6BundleAgeCanonicalBlock : public Bpv6CanonicalBlock {
    static constexpr uint64_t largestSerializedDataOnlySize = 10; //sdnv bundle age

    Bpv6BundleAgeCanonicalBlock(); //a default constructor: X()
    virtual ~Bpv6BundleAgeCanonicalBlock(); //a destructor: ~X()
    Bpv6BundleAgeCanonicalBlock(const Bpv6BundleAgeCanonicalBlock& o); //a copy constructor: X(const X&)
    Bpv6BundleAgeCanonicalBlock(Bpv6BundleAgeCanonicalBlock&& o); //a move constructor: X(X&&)
    Bpv6BundleAgeCanonicalBlock& operator=(const Bpv6BundleAgeCanonicalBlock& o); //a copy assignment: operator=(const X&)
    Bpv6BundleAgeCanonicalBlock& operator=(Bpv6BundleAgeCanonicalBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6BundleAgeCanonicalBlock & o) const; //operator ==
    bool operator!=(const Bpv6BundleAgeCanonicalBlock & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_dataPtr to serialized location
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();

    uint64_t m_bundleAgeMicroseconds;
};

enum class BPV6_METADATA_TYPE_CODE : uint64_t {
    UNDEFINED_ZERO = 0,
    URI = 1
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_METADATA_TYPE_CODE);

struct Bpv6MetadataContentBase {
    virtual ~Bpv6MetadataContentBase() = 0; // Pure virtual destructor
    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const = 0;
    virtual uint64_t GetSerializationSize() const = 0;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize) = 0;
    virtual bool IsEqual(const Bpv6MetadataContentBase * otherPtr) const = 0;
};

class Bpv6MetadataContentUriList : public Bpv6MetadataContentBase {
public:
    std::vector<cbhe_eid_t> m_uriArray;

public:
    Bpv6MetadataContentUriList(); //a default constructor: X()
    virtual ~Bpv6MetadataContentUriList(); //a destructor: ~X()
    Bpv6MetadataContentUriList(const Bpv6MetadataContentUriList& o); //a copy constructor: X(const X&)
    Bpv6MetadataContentUriList(Bpv6MetadataContentUriList&& o); //a move constructor: X(X&&)
    Bpv6MetadataContentUriList& operator=(const Bpv6MetadataContentUriList& o); //a copy assignment: operator=(const X&)
    Bpv6MetadataContentUriList& operator=(Bpv6MetadataContentUriList&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6MetadataContentUriList & o) const;
    bool operator!=(const Bpv6MetadataContentUriList & o) const;

    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const ;
    virtual uint64_t GetSerializationSize() const;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    virtual bool IsEqual(const Bpv6MetadataContentBase * otherPtr) const;

    void Reset();
};

class Bpv6MetadataContentGeneric : public Bpv6MetadataContentBase {
public:
    std::vector<uint8_t> m_genericRawMetadata;

public:
    Bpv6MetadataContentGeneric(); //a default constructor: X()
    virtual ~Bpv6MetadataContentGeneric(); //a destructor: ~X()
    Bpv6MetadataContentGeneric(const Bpv6MetadataContentGeneric& o); //a copy constructor: X(const X&)
    Bpv6MetadataContentGeneric(Bpv6MetadataContentGeneric&& o); //a move constructor: X(X&&)
    Bpv6MetadataContentGeneric& operator=(const Bpv6MetadataContentGeneric& o); //a copy assignment: operator=(const X&)
    Bpv6MetadataContentGeneric& operator=(Bpv6MetadataContentGeneric&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6MetadataContentGeneric & o) const;
    bool operator!=(const Bpv6MetadataContentGeneric & o) const;

    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const;
    virtual uint64_t GetSerializationSize() const;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    virtual bool IsEqual(const Bpv6MetadataContentBase * otherPtr) const;

    void Reset();
};

//https://datatracker.ietf.org/doc/html/rfc6258
struct Bpv6MetadataCanonicalBlock : public Bpv6CanonicalBlock {

    Bpv6MetadataCanonicalBlock(); //a default constructor: X()
    virtual ~Bpv6MetadataCanonicalBlock(); //a destructor: ~X()
    Bpv6MetadataCanonicalBlock(const Bpv6MetadataCanonicalBlock& o) = delete; //a copy constructor: X(const X&)
    Bpv6MetadataCanonicalBlock(Bpv6MetadataCanonicalBlock&& o); //a move constructor: X(X&&)
    Bpv6MetadataCanonicalBlock& operator=(const Bpv6MetadataCanonicalBlock& o) = delete; //a copy assignment: operator=(const X&)
    Bpv6MetadataCanonicalBlock& operator=(Bpv6MetadataCanonicalBlock&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6MetadataCanonicalBlock & o) const; //operator ==
    bool operator!=(const Bpv6MetadataCanonicalBlock & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_dataPtr to serialized location
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();

    BPV6_METADATA_TYPE_CODE m_metadataTypeCode;
    std::unique_ptr<Bpv6MetadataContentBase> m_metadataContentPtr;
};

//Administrative record types
enum class BPV6_ADMINISTRATIVE_RECORD_TYPE_CODE : uint8_t {
    UNUSED_ZERO              = 0,
    BUNDLE_STATUS_REPORT     = 1,
    CUSTODY_SIGNAL           = 2,
    AGGREGATE_CUSTODY_SIGNAL = 4,
    ENCAPSULATED_BUNDLE      = 7,
    SAGA_MESSAGE             = 42
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_ADMINISTRATIVE_RECORD_TYPE_CODE);

//Administrative record flags
enum class BPV6_ADMINISTRATIVE_RECORD_FLAGS : uint8_t {
    BUNDLE_IS_A_FRAGMENT = 1 //00000001
};
MAKE_ENUM_SUPPORT_FLAG_OPERATORS(BPV6_ADMINISTRATIVE_RECORD_FLAGS);
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_ADMINISTRATIVE_RECORD_FLAGS);


enum class BPV6_BUNDLE_STATUS_REPORT_STATUS_FLAGS : uint8_t {
    NO_FLAGS_SET                                 = 0,
    REPORTING_NODE_RECEIVED_BUNDLE               = (1 << 0),
    REPORTING_NODE_ACCEPTED_CUSTODY_OF_BUNDLE    = (1 << 1),
    REPORTING_NODE_FORWARDED_BUNDLE              = (1 << 2),
    REPORTING_NODE_DELIVERED_BUNDLE              = (1 << 3),
    REPORTING_NODE_DELETED_BUNDLE                = (1 << 4),
};
MAKE_ENUM_SUPPORT_FLAG_OPERATORS(BPV6_BUNDLE_STATUS_REPORT_STATUS_FLAGS);
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_BUNDLE_STATUS_REPORT_STATUS_FLAGS);

enum class BPV6_BUNDLE_STATUS_REPORT_REASON_CODES : uint8_t {
    NO_ADDITIONAL_INFORMATION                   = 0,
    LIFETIME_EXPIRED                            = 1,
    FORWARDED_OVER_UNIDIRECTIONAL_LINK          = 2,
    TRANSMISSION_CANCELLED                      = 3,
    DEPLETED_STORAGE                            = 4,
    DESTINATION_ENDPOINT_ID_UNINTELLIGIBLE      = 5,
    NO_KNOWN_ROUTE_TO_DESTINATION_FROM_HERE     = 6,
    NO_TIMELY_CONTACT_WITH_NEXT_NODE_ON_ROUTE   = 7,
    BLOCK_UNINTELLIGIBLE                        = 8
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_BUNDLE_STATUS_REPORT_REASON_CODES);

enum class BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT : uint8_t {
    NO_ADDITIONAL_INFORMATION                   = 0,
    REDUNDANT_RECEPTION                         = 3,
    DEPLETED_STORAGE                            = 4,
    DESTINATION_ENDPOINT_ID_UNINTELLIGIBLE      = 5,
    NO_KNOWN_ROUTE_TO_DESTINATION_FROM_HERE     = 6,
    NO_TIMELY_CONTACT_WITH_NEXT_NODE_ON_ROUTE   = 7,
    BLOCK_UNINTELLIGIBLE                        = 8
};
MAKE_ENUM_SUPPORT_OSTREAM_OPERATOR(BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT);


struct Bpv6AdministrativeRecordContentBase {
    virtual ~Bpv6AdministrativeRecordContentBase() = 0; // Pure virtual destructor
    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const = 0;
    virtual uint64_t GetSerializationSize() const = 0;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize) = 0;
    virtual bool IsEqual(const Bpv6AdministrativeRecordContentBase * otherPtr) const = 0;
};

class Bpv6AdministrativeRecordContentBundleStatusReport : public Bpv6AdministrativeRecordContentBase {


public:
    static constexpr unsigned int CBHE_MAX_SERIALIZATION_SIZE =
        3 + //admin flags + status flags + reason code
        10 + //fragmentOffsetSdnv.length +
        10 + //fragmentLengthSdnv.length +
        10 + //receiptTimeSecondsSdnv.length +
        5 + //receiptTimeNanosecSdnv.length +
        10 + //custodyTimeSecondsSdnv.length +
        5 + //custodyTimeNanosecSdnv.length +
        10 + //forwardTimeSecondsSdnv.length +
        5 + //forwardTimeNanosecSdnv.length +
        10 + //deliveryTimeSecondsSdnv.length +
        5 + //deliveryTimeNanosecSdnv.length +
        10 + //deletionTimeSecondsSdnv.length +
        5 + //deletionTimeNanosecSdnv.length +
        10 + //creationTimeSecondsSdnv.length +
        10 + //creationTimeCountSdnv.length +
        1 + //eidLengthSdnv.length +
        45; //length of "ipn:18446744073709551615.18446744073709551615" (note 45 > 32 so sdnv hardware acceleration overwrite is satisfied)
    BPV6_BUNDLE_STATUS_REPORT_STATUS_FLAGS m_statusFlags;
    BPV6_BUNDLE_STATUS_REPORT_REASON_CODES m_reasonCode;
    bool m_isFragment;
    uint64_t m_fragmentOffsetIfPresent;
    uint64_t m_fragmentLengthIfPresent;

    TimestampUtil::dtn_time_t m_timeOfReceiptOfBundle;
    TimestampUtil::dtn_time_t m_timeOfCustodyAcceptanceOfBundle;
    TimestampUtil::dtn_time_t m_timeOfForwardingOfBundle;
    TimestampUtil::dtn_time_t m_timeOfDeliveryOfBundle;
    TimestampUtil::dtn_time_t m_timeOfDeletionOfBundle;

    //from primary block of subject bundle
    TimestampUtil::bpv6_creation_timestamp_t m_copyOfBundleCreationTimestamp;

    std::string m_bundleSourceEid;

public:
    Bpv6AdministrativeRecordContentBundleStatusReport(); //a default constructor: X()
    virtual ~Bpv6AdministrativeRecordContentBundleStatusReport(); //a destructor: ~X()
    Bpv6AdministrativeRecordContentBundleStatusReport(const Bpv6AdministrativeRecordContentBundleStatusReport& o); //a copy constructor: X(const X&)
    Bpv6AdministrativeRecordContentBundleStatusReport(Bpv6AdministrativeRecordContentBundleStatusReport&& o); //a move constructor: X(X&&)
    Bpv6AdministrativeRecordContentBundleStatusReport& operator=(const Bpv6AdministrativeRecordContentBundleStatusReport& o); //a copy assignment: operator=(const X&)
    Bpv6AdministrativeRecordContentBundleStatusReport& operator=(Bpv6AdministrativeRecordContentBundleStatusReport&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6AdministrativeRecordContentBundleStatusReport & o) const;
    bool operator!=(const Bpv6AdministrativeRecordContentBundleStatusReport & o) const;

    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const ;
    virtual uint64_t GetSerializationSize() const;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    virtual bool IsEqual(const Bpv6AdministrativeRecordContentBase * otherPtr) const;

    void Reset();

    void SetTimeOfReceiptOfBundleAndStatusFlag(const TimestampUtil::dtn_time_t & dtnTime);
    void SetTimeOfCustodyAcceptanceOfBundleAndStatusFlag(const TimestampUtil::dtn_time_t & dtnTime);
    void SetTimeOfForwardingOfBundleAndStatusFlag(const TimestampUtil::dtn_time_t & dtnTime);
    void SetTimeOfDeliveryOfBundleAndStatusFlag(const TimestampUtil::dtn_time_t & dtnTime);
    void SetTimeOfDeletionOfBundleAndStatusFlag(const TimestampUtil::dtn_time_t & dtnTime);
    bool HasBundleStatusReportStatusFlagSet(const BPV6_BUNDLE_STATUS_REPORT_STATUS_FLAGS & flag) const;
};

class Bpv6AdministrativeRecordContentCustodySignal : public Bpv6AdministrativeRecordContentBase {
    

public:
    static constexpr unsigned int CBHE_MAX_SERIALIZATION_SIZE =
        2 + //admin flags + (bit7 status flags |  bit 6..0 reason code)
        10 + //fragmentOffsetSdnv.length +
        10 + //fragmentLengthSdnv.length +
        10 + //signalTimeSecondsSdnv.length +
        5 + //signalTimeNanosecSdnv.length +
        10 + //creationTimeSecondsSdnv.length +
        10 + //creationTimeCountSdnv.length +
        1 + //eidLengthSdnv.length +
        45; //length of "ipn:18446744073709551615.18446744073709551615" (note 45 > 32 so sdnv hardware acceleration overwrite is satisfied)
private:
    uint8_t m_statusFlagsPlus7bitReasonCode;
public:
    
    bool m_isFragment;
    uint64_t m_fragmentOffsetIfPresent;
    uint64_t m_fragmentLengthIfPresent;

    TimestampUtil::dtn_time_t m_timeOfSignalGeneration;

    //from primary block of subject bundle
    TimestampUtil::bpv6_creation_timestamp_t m_copyOfBundleCreationTimestamp;

    std::string m_bundleSourceEid;

public:
    Bpv6AdministrativeRecordContentCustodySignal(); //a default constructor: X()
    virtual ~Bpv6AdministrativeRecordContentCustodySignal(); //a destructor: ~X()
    Bpv6AdministrativeRecordContentCustodySignal(const Bpv6AdministrativeRecordContentCustodySignal& o); //a copy constructor: X(const X&)
    Bpv6AdministrativeRecordContentCustodySignal(Bpv6AdministrativeRecordContentCustodySignal&& o); //a move constructor: X(X&&)
    Bpv6AdministrativeRecordContentCustodySignal& operator=(const Bpv6AdministrativeRecordContentCustodySignal& o); //a copy assignment: operator=(const X&)
    Bpv6AdministrativeRecordContentCustodySignal& operator=(Bpv6AdministrativeRecordContentCustodySignal&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6AdministrativeRecordContentCustodySignal & o) const;
    bool operator!=(const Bpv6AdministrativeRecordContentCustodySignal & o) const;

    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const;
    virtual uint64_t GetSerializationSize() const;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    virtual bool IsEqual(const Bpv6AdministrativeRecordContentBase * otherPtr) const;

    void Reset();

    void SetTimeOfSignalGeneration(const TimestampUtil::dtn_time_t & dtnTime);
    void SetCustodyTransferStatusAndReason(bool custodyTransferSucceeded, BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT reasonCode7bit);
    bool DidCustodyTransferSucceed() const;
    BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT GetReasonCode() const;
};

class Bpv6AdministrativeRecordContentAggregateCustodySignal : public Bpv6AdministrativeRecordContentBase {
    
private:
    //The second field shall be a �Status� byte encoded in the same way as the status byte
    //for administrative records in RFC 5050, using the same reason codes
    uint8_t m_statusFlagsPlus7bitReasonCode;
public:
    std::set<FragmentSet::data_fragment_t> m_custodyIdFills;

public:
    Bpv6AdministrativeRecordContentAggregateCustodySignal(); //a default constructor: X()
    virtual ~Bpv6AdministrativeRecordContentAggregateCustodySignal(); //a destructor: ~X()
    Bpv6AdministrativeRecordContentAggregateCustodySignal(const Bpv6AdministrativeRecordContentAggregateCustodySignal& o); //a copy constructor: X(const X&)
    Bpv6AdministrativeRecordContentAggregateCustodySignal(Bpv6AdministrativeRecordContentAggregateCustodySignal&& o); //a move constructor: X(X&&)
    Bpv6AdministrativeRecordContentAggregateCustodySignal& operator=(const Bpv6AdministrativeRecordContentAggregateCustodySignal& o); //a copy assignment: operator=(const X&)
    Bpv6AdministrativeRecordContentAggregateCustodySignal& operator=(Bpv6AdministrativeRecordContentAggregateCustodySignal&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6AdministrativeRecordContentAggregateCustodySignal & o) const;
    bool operator!=(const Bpv6AdministrativeRecordContentAggregateCustodySignal & o) const;

    virtual uint64_t SerializeBpv6(uint8_t * serialization, uint64_t bufferSize) const;
    virtual uint64_t GetSerializationSize() const;
    virtual bool DeserializeBpv6(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
    virtual bool IsEqual(const Bpv6AdministrativeRecordContentBase * otherPtr) const;

    void Reset();

    
    void SetCustodyTransferStatusAndReason(bool custodyTransferSucceeded, BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT reasonCode7bit);
    bool DidCustodyTransferSucceed() const;
    BPV6_CUSTODY_SIGNAL_REASON_CODES_7BIT GetReasonCode() const;
    //return number of fills
    uint64_t AddCustodyIdToFill(const uint64_t custodyId);
    //return number of fills
    uint64_t AddContiguousCustodyIdsToFill(const uint64_t firstCustodyId, const uint64_t lastCustodyId);
public: //only public for unit testing
    uint64_t SerializeFills(uint8_t * serialization, uint64_t bufferSize) const;
    uint64_t GetFillSerializedSize() const;
    bool DeserializeFills(const uint8_t * serialization, uint64_t & numBytesTakenToDecode, uint64_t bufferSize);
};

struct Bpv6AdministrativeRecord : public Bpv6CanonicalBlock {

    BPV6_ADMINISTRATIVE_RECORD_TYPE_CODE m_adminRecordTypeCode;
    std::unique_ptr<Bpv6AdministrativeRecordContentBase> m_adminRecordContentPtr;
    bool m_isFragment;

    Bpv6AdministrativeRecord(); //a default constructor: X()
    virtual ~Bpv6AdministrativeRecord(); //a destructor: ~X()
    Bpv6AdministrativeRecord(const Bpv6AdministrativeRecord& o) = delete;; //a copy constructor: X(const X&)
    Bpv6AdministrativeRecord(Bpv6AdministrativeRecord&& o); //a move constructor: X(X&&)
    Bpv6AdministrativeRecord& operator=(const Bpv6AdministrativeRecord& o) = delete;; //a copy assignment: operator=(const X&)
    Bpv6AdministrativeRecord& operator=(Bpv6AdministrativeRecord&& o); //a move assignment: operator=(X&&)
    bool operator==(const Bpv6AdministrativeRecord & o) const; //operator ==
    bool operator!=(const Bpv6AdministrativeRecord & o) const; //operator !=
    virtual void SetZero();
    virtual uint64_t SerializeBpv6(uint8_t * serialization); //modifies m_dataPtr to serialized location
    virtual uint64_t GetCanonicalBlockTypeSpecificDataSerializationSize() const;
    virtual bool Virtual_DeserializeExtensionBlockDataBpv6();
};



#endif //BPV6_H
