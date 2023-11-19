<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.9.1">
  <compound kind="file">
    <name>decoder.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC++/</path>
    <filename>decoder_8h.html</filename>
    <includes id="FLAC_09_09_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="stream__decoder_8h" name="stream_decoder.h" local="yes" imported="no">FLAC/stream_decoder.h</includes>
    <class kind="class">FLAC::Decoder::Stream</class>
    <class kind="class">FLAC::Decoder::Stream::State</class>
    <class kind="class">FLAC::Decoder::File</class>
  </compound>
  <compound kind="file">
    <name>encoder.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC++/</path>
    <filename>encoder_8h.html</filename>
    <includes id="FLAC_09_09_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="stream__encoder_8h" name="stream_encoder.h" local="yes" imported="no">FLAC/stream_encoder.h</includes>
    <includes id="decoder_8h" name="decoder.h" local="yes" imported="no">decoder.h</includes>
    <includes id="FLAC_09_09_2metadata_8h" name="metadata.h" local="yes" imported="no">metadata.h</includes>
    <class kind="class">FLAC::Encoder::Stream</class>
    <class kind="class">FLAC::Encoder::Stream::State</class>
    <class kind="class">FLAC::Encoder::File</class>
  </compound>
  <compound kind="file">
    <name>callback.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>callback_8h.html</filename>
    <class kind="struct">FLAC__IOCallbacks</class>
    <member kind="typedef">
      <type>void *</type>
      <name>FLAC__IOHandle</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga4c329c3168dee6e352384c5e9306260d</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>size_t(*</type>
      <name>FLAC__IOCallback_Read</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga49d95218a6c09b215cd92cc96de71bf9</anchor>
      <arglist>)(void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>size_t(*</type>
      <name>FLAC__IOCallback_Write</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gad991792235879aecae289b56a112e1b8</anchor>
      <arglist>)(const void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Seek</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gab3942bbbd6ae09bcefe7cb3a0060c49c</anchor>
      <arglist>)(FLAC__IOHandle handle, FLAC__int64 offset, int whence)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__int64(*</type>
      <name>FLAC__IOCallback_Tell</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga45314930cabc2e9c04867eae6bca309f</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Eof</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga00ae3b3d373e691908e9539ebf720675</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Close</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga0032267fac38220689778833e08f7387</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>export.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>FLAC_2export_8h.html</filename>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga56ca07df8a23310707732b1c0007d6f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_CURRENT</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga31180fe15eea416cd8957cfca1a4c4f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_REVISION</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga811641dd9f8c542d9260240e7fbe8e93</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_AGE</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga1add3e09c8dfd57e8c921f299f0bbec1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>FLAC_API_SUPPORTS_OGG_FLAC</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga84ffcb0af1038c60eb3e21fd002093cf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>export.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC++/</path>
    <filename>FLAC_09_09_2export_8h.html</filename>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gaec3a801bf18630403eda6dc2f8c4927a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_CURRENT</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gafc3064beba20c1795d8aaa801b79d3b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_REVISION</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gaebce36e5325dbdcdc1a9e61a44606efe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_AGE</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>ga17d0e89a961696b32c2b11e08663543f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>format.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>format_8h.html</filename>
    <includes id="FLAC_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <class kind="struct">FLAC__EntropyCodingMethod_PartitionedRiceContents</class>
    <class kind="struct">FLAC__EntropyCodingMethod_PartitionedRice</class>
    <class kind="struct">FLAC__EntropyCodingMethod</class>
    <class kind="struct">FLAC__Subframe_Constant</class>
    <class kind="struct">FLAC__Subframe_Verbatim</class>
    <class kind="struct">FLAC__Subframe_Fixed</class>
    <class kind="struct">FLAC__Subframe_LPC</class>
    <class kind="struct">FLAC__Subframe</class>
    <class kind="struct">FLAC__FrameHeader</class>
    <class kind="struct">FLAC__FrameFooter</class>
    <class kind="struct">FLAC__Frame</class>
    <class kind="struct">FLAC__StreamMetadata_StreamInfo</class>
    <class kind="struct">FLAC__StreamMetadata_Padding</class>
    <class kind="struct">FLAC__StreamMetadata_Application</class>
    <class kind="struct">FLAC__StreamMetadata_SeekPoint</class>
    <class kind="struct">FLAC__StreamMetadata_SeekTable</class>
    <class kind="struct">FLAC__StreamMetadata_VorbisComment_Entry</class>
    <class kind="struct">FLAC__StreamMetadata_VorbisComment</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet_Index</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet_Track</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet</class>
    <class kind="struct">FLAC__StreamMetadata_Picture</class>
    <class kind="struct">FLAC__StreamMetadata_Unknown</class>
    <class kind="struct">FLAC__StreamMetadata</class>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_METADATA_TYPE_CODE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga626a412545818c2271fa2202c02ff1d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_BLOCK_SIZE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa5a85c2ea434221ce684be3469517003</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_BLOCK_SIZE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaef78bc1b04f721e7b4563381f5514e8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_BLOCK_SIZE_48000HZ</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8f6ba2c28fbfcf52326d115c95b0a751</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_CHANNELS</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga488aa5678a58d08f984f5d39185b763d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga30b0f21abbb2cdfd461fe04b425b5438</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad0156d56751e80241fa349d1e25064a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__REFERENCE_CODEC_MAX_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0fc418d96053d385fd2f56dce8007fbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_SAMPLE_RATE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga99abeef0c05c6bc76eacfa865abbfa70</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_LPC_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga16108d413f524329f338cff6e05f3aff</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_LPC_ORDER_48000HZ</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9791efa78147196820c86a6041d7774d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf52033b2950b9396dd92b167b3bbe4db</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6aa38a4bc5b9d96a78253ccb8b08bd1f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_FIXED_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabd0d5d6fe71b337244712b244ae7cb0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_RICE_PARTITION_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga78a2e97e230b2aa7f99edc94a466f5bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_RICE_PARTITION_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab19dec1b56de482ccfeb5f9843f60a14</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_SYNC_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae7ddaf298d3ceb83aae6301908675c1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga06dfae7260da40e4c5f8fc4d531b326c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabdf85aa2c9a483378dfe850b85ab93ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_HEADER_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga706a29b8a14902c457783bfd4fd7bab2</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct FLAC__StreamMetadata</type>
      <name>FLAC__StreamMetadata</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga37ced8d328607ea72b2e51c8ef9e2e58</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__EntropyCodingMethodType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga951733d2ea01943514290012cd622d3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa5253f8b8edc61220739f229a299775dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa202960a608ee91f9f11c2575b9ecc5aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__SubframeType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1f431eaf213e74d7747589932d263348</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_CONSTANT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a9bf56d836aeffb11d614e29ea1cdf2a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_VERBATIM</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a8520596ef07d6c8577f07025f137657b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_FIXED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a6b3cce73039a513f9afefdc8e4f664a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_LPC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a31437462c3e4c3a5a214a91eff8cc3af</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__VerbatimSubframeDataType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabf8b5851429eae13f26267bafe7c5d32</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32a9c1ed26317d9c2fe252bc92a4d1c6e4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32aaf4bfde2c07ab557250a2bdc63e7ad6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__ChannelAssignment</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga79855f8525672e37f299bbe02952ef9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca3c554e4c8512c2de31dfd3305f8b31b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca28d41295b20593561dc9934cc977d5cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9cad155b61582140b2b90362005f1a93e2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_MID_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca85c1512c0473b5ede364a9943759a80c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__FrameNumberType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8fe9ebc78386cd2a3d23b7b8e3818e1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca0b9cbf3853f0ae105cf9b5360164f794</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca9220ce93dcc151e5edd5db7e7155b35a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__MetadataType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac71714ba8ddbbd66d26bb78a427fac01</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_STREAMINFO</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acffa517e969ba6a868dcf10e5da75c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PADDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a6dcb741fc0aef389580f110e88beb896</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_APPLICATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a2b287a22a1ac9440b309127884c8d41b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_SEEKTABLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a5f6323e489be1318f0e3747960ebdd91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_VORBIS_COMMENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01ad013576bc5196b907547739518605520</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_CUESHEET</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a0b3f07ae60609126562cd0233ce00a65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PICTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf28ae2788366617c1aeab81d5961c6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_UNDEFINED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf6ac61fcc866608f5583c275dc34d47</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__MAX_METADATA_TYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a1a2f283a3dd9e7b46181d7a114ec5805</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamMetadata_Picture_Type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf6d3e836cee023e0b8d897f1fdc9825d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_OTHER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dadd6d6af32499b1973e48c9e8f13357ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON_STANDARD</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5eca52e5cfcb718f33f5fce9b1021a49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825daaf44b9d5fb75dde6941463e5029aa351</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da3e20b405fd4e835ff3a4465b8bcb7c36</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BACK_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da9ae132f2ee7d3baf35f94a9dc9640f62</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAFLET_PAGE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dad3cb471b7925ae5034d9fd9ecfafb87a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_MEDIA</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac994edc4166107ab5790e49f0b57ffd9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAD_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da1282e252e20553c39907074052960f42</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4cead70f8720f180fc220e6df8d55cce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_CONDUCTOR</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dae01a47af0b0c4d89500b755ebca866ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8515523b4c9ab65ffef7db98bc09ceb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_COMPOSER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5ea1554bc96deb45731bc5897600d1c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LYRICIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da86159eda8969514f5992b3e341103f22</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_RECORDING_LOCATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac96e810cdd81465709b4a3a03289e89c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_RECORDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8cee3bb376ed1044b3a7e20b9c971ff1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_PERFORMANCE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4d4dc6904984370501865988d948de3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_VIDEO_SCREEN_CAPTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da7adc2b194968b51768721de7bda39df9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FISH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dabbf0d7c519ae8ba8cec7d1f165f67b0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ILLUSTRATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da89ba412c9d89c937c28afdab508d047a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da751716a4528a78a8d53f435c816c4917</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_PUBLISHER_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da31d75150a4079482fe122e703eff9141</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_sample_rate_is_valid</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga48100669b8e8613f1e226c3925f701a8</anchor>
      <arglist>(uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_blocksize_is_subset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4e71651ff9b90b50480f86050d78c16b</anchor>
      <arglist>(uint32_t blocksize, uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_sample_rate_is_subset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae048df385980088b4c29c52aa7207306</anchor>
      <arglist>(uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_name_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae5fb55cd5977ebf178c5b38da831c057</anchor>
      <arglist>(const char *name)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_value_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1a5061a12c836cc2ff3967088afda1c4</anchor>
      <arglist>(const FLAC__byte *value, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1439057dbc3f0719309620caaf82c1b1</anchor>
      <arglist>(const FLAC__byte *entry, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_seektable_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga02ed0843553fb8f718fe8e7c54d12244</anchor>
      <arglist>(const FLAC__StreamMetadata_SeekTable *seek_table)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__format_seektable_sort</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2285adb37d91c41b1f9a5c3b1b35e886</anchor>
      <arglist>(FLAC__StreamMetadata_SeekTable *seek_table)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_cuesheet_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa9ed0fa4ed04dbfdaa163d0f5308c080</anchor>
      <arglist>(const FLAC__StreamMetadata_CueSheet *cue_sheet, FLAC__bool check_cd_da_subset, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_picture_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga82ca3ffc97c106c61882134f1a7fb1be</anchor>
      <arglist>(const FLAC__StreamMetadata_Picture *picture, const char **violation)</arglist>
    </member>
    <member kind="variable">
      <type>const char *</type>
      <name>FLAC__VERSION_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga52e2616f9a0b94881cd7711c18d62a35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *</type>
      <name>FLAC__VENDOR_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad5cccab0de3adda58914edf3c31fd64f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__byte</type>
      <name>FLAC__STREAM_SYNC_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3f275a3a6056e0d53df3b72b03adde4b</anchor>
      <arglist>[4]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_SYNC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf836406a1f4c1b37ef6e4023f65c127f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_SYNC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa95eb3cb07b7d503de94521a155af6bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__EntropyCodingMethodTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga41603ac35eed8c77c2f2e0b12067d88a</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga12fe0569d6d11d6e6ba8d3342196ccc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0c00e7f349eabc3d25dab7223cc5af15</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2_PARAMETER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6d5cfd610e45402ac02d5786bda8a755</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7aed9c761b806bfd787c077da0ab9a07</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga80fb6cc2fb05edcea2a7e3ae004096a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2_ESCAPE_PARAMETER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga12e2bed2777e9beb187498ca116bcb0a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga18e9f8910a79bebe138a76a1a923076f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__SubframeTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga78d78f45f123cfbb50cebd61b96097df</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga303c4e38674249f42ec8735354622463</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga918e00beab5d7826e37b6397520df4c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_ZERO_PAD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8f4ad64ca91dd750a38b5c2d30838fdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga65c51d6c43f33179072d7225768e14a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf2e0e7e4f28e357646ad7e5dfcc90f2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_CONSTANT_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb235be931ef14cee71ad37bc1924667</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_VERBATIM_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga93b8d9b7b76ff5cefa8ce8965a9dca9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_FIXED_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac7884342f77d4f16f1921a0cc7a2d3ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_LPC_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5c1baa1525de2749f74c174fad422266</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__ChannelAssignmentString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab1a1d3929a4e5a5aff2c15010742aa21</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__FrameNumberTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga931a0e63c0f2b31fab801e1dd693fa4e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SYNC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7af18147ae3a5bb75136843f6e271a4d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SYNC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab3821624c367fac8d994d0ab43229c13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaed36cf061a5112a72d33b5fdb2941cf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BLOCKING_STRATEGY_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga73711753949d786e168222b2cf9502dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf9b185ee73ab9166498aa087f506c895</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SAMPLE_RATE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8c686e8933c321c9d386db6a6f0d5f70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_CHANNEL_ASSIGNMENT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8d2909446c32443619b9967188a07fb7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BITS_PER_SAMPLE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga47f63b74fff6e3396d6203d1022062be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_ZERO_PAD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3d73f3519e9ec387c1cf5d54bdfb022f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_CRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac0478a55947c6fb97f53f6a9222a0952</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_FOOTER_CRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3e74578ca10d5a2a80766040443665f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__MetadataTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa9ad23f06a579d1110d61d54c8c999f0</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga08f9ac0cd9e3fe8db67a16c011b1c9f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga60a3c8fc22960cec9adb6e22b866d61c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaab054a54f7725f6fc250321f245e1f9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gafb35eac8504f1903654cb28f924c5c22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaac031487db3e1961cb5d48f0ce5107b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab7c3111fe0e73ac3b323ba881d02a8b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaae73b50a208bc0b9479b56b5be546f69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0d6496e976945999313c9029dba46b2b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MD5SUM_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga651ba492225f315a70286eccd3c3184b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_APPLICATION_ID_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8040c7fa72cfc55c74e43d620e64a805</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9e95bd97ef2fa28b1d5bbd3917160f9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaaa177c78a35cdd323845928326274f63</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_FRAME_SAMPLES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga62341e0615038b3eade3c7691f410cca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__uint64</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_PLACEHOLDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad5d58774aea926635e6841c411d60566</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7ff8c3f4693944031b9ac8ff99093df6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_VORBIS_COMMENT_NUM_COMMENTS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2019f140758b10d086e438e43a257036</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab448a7b0ee7c06c6fa23155d29c37ccb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9d3b4268a36fa8a5d5f8cf2ee704ceb2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga978b9c0ec4220d22a6bd4aab75fb9949</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad09fd65eb06250d671d05eb8e999cc89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac4fb0980ac6a409916e4122ba25ae8fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_ISRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga76dc2c2ae2385f2ab0752f16f7f9d4c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf7f2927d240eeab1214a88bceb5deae6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_PRE_EMPHASIS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga715d4e09605238e3b40afdbdaf4717b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga06b1d7142a95fa837eff737ee8f825be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_NUM_INDICES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4b4231131e11b216e34e49d12f210363</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_MEDIA_CATALOG_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaae2030a18d8421dc476ff18c95f773d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_LEAD_IN_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga397890e4c43ca950d2236250d69a92f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_IS_CD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga285c570708526c7ebcb742c982e5d5fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb9458a79b7d214e8758cc5ad4e2b18a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_NUM_TRACKS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa30d6a1d38397b4851add1bb2a6d145c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamMetadata_Picture_TypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2d27672452696cb97fd39db1cf43486b</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9a91512adcf0f8293c0a8793ce8b246c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_MIME_TYPE_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5186600f0920191cb61e55b2c7628287</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DESCRIPTION_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6d71497d949952f8d8b16f482ebcf555</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_WIDTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2819d0e2a032fd5947a1259e40b5f52a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_HEIGHT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf537b699909721adca031b6e3826ce22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DEPTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga553826edf5d175f81f162e3049c386ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_COLORS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3f810c75aad1f5a0c9d1d85c56998b5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DATA_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gafd1dd421206189d123f644ff3717cb12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_IS_LAST_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa51331191b62fb15793b0a35ea8821e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaec6fd2f0de2c3f88b7bb0449d178043c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga90cbf669f1c3400813ee4ecdd3462ca3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>metadata.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>FLAC_2metadata_8h.html</filename>
    <includes id="FLAC_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="callback_8h" name="callback.h" local="yes" imported="no">callback.h</includes>
    <includes id="format_8h" name="format.h" local="yes" imported="no">format.h</includes>
    <member kind="typedef">
      <type>struct FLAC__Metadata_SimpleIterator</type>
      <name>FLAC__Metadata_SimpleIterator</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga6accccddbb867dfc2eece9ee3ffecb3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct FLAC__Metadata_Chain</type>
      <name>FLAC__Metadata_Chain</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaec6993c60b88f222a52af86f8f47bfdf</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct FLAC__Metadata_Iterator</type>
      <name>FLAC__Metadata_Iterator</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga9f3e135a07cdef7e51597646aa7b89b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__Metadata_SimpleIteratorStatus</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gac926e7d2773a05066115cac9048bbec9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a33aadd73194c0d7e307d643237e0ddcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a0a3933cb38c8957a8d5c3d1afb4766f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a20e835bbb74b4d039e598617f68d2af6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a7785f77a612be8956fbe7cab73497220</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9af055d8c0c663e72134fe2db8037b6880</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a14c897124887858109200723826f85b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a088df964f0852dd7e19304e920c3ee8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2ad85a32e291d1e918692d68cc22fd40</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9ac2337299c2347ca311caeaa7d71d857c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2e073843fa99419d76a0b210da96ceb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a4f855433038c576da127fc1de9d18f9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9aa8386ed0a20d7e91b0022d203ec3cdec</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a9d821ae65a1c5de619daa88c850906df</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__Metadata_ChainStatus</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gafe2a924893b0800b020bea8160fd4531</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a293be942ec54576f2b3c73613af968e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1be9400982f411173af46bf0c3acbdc7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a43d2741a650576052fa3615d8cd64d86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a99748a4b12ed10f9368375cc8deeb143</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ac469c6543ebb117e99064572c16672d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a8efd2c76dc06308eb6eba59e1bc6300b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0525de5fb5d8aeeb4e848e33a8d503c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a5814bc26bcf92143198b8e7f028f43a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a66460c735e4745788b40889329e8489f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af4ecf22bc3e5adf78a9c765f856efb0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1cd3138ed493f6a0f5b95fb8481edd1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab12ec938f7556a163c609194ee0aede0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a36b9bcf93da8e0f111738a65eab36e9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab8a6aa5f115db3f07ad2ed4adbcbe060</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_WRITE_MISMATCH</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0d9e64ad6514c88b8ea9e9171c42ec9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRONG_WRITE_CALL</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af86670707345e2d02cc84aec059459d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_streaminfo</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga804b42d9da714199b4b383ce51078d51</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata *streaminfo)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_tags</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga1626af09cd39d4fa37d5b46ebe3790fd</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **tags)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_cuesheet</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga0f47949dca514506718276205a4fae0b</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_picture</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>gab9f69e48c5a33cacb924d13986bfb852</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **picture, FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_SimpleIterator *</type>
      <name>FLAC__metadata_simple_iterator_new</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga017ae86f3351888f50feb47026ed2482</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_simple_iterator_delete</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga4619be06f51429fea71e5b98900cec3e</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_SimpleIteratorStatus</type>
      <name>FLAC__metadata_simple_iterator_status</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gae8fd236fe6049c61f7f3b4a6ecbcd240</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_init</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaba8daf276fd7da863a2522ac050125fd</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, const char *filename, FLAC__bool read_only, FLAC__bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_is_writable</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga5150ecd8668c610f79192a2838667790</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_next</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gabb7de0a1067efae353e0792dc6e51905</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_prev</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga6db5313b31120b28e210ae721d6525a8</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_is_last</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga9eb215059840960de69aa84469ba954f</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>off_t</type>
      <name>FLAC__metadata_simple_iterator_get_block_offset</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gade0a61723420daeb4bc226713671c6f0</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__MetadataType</type>
      <name>FLAC__metadata_simple_iterator_get_block_type</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga17b61d17e83432913abf4334d6e0c073</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__metadata_simple_iterator_get_block_length</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaf29b9a7f2e2c762756c1444e55a119fa</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_get_application_id</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gad4fea2d7d98d16e75e6d8260f690a5dc</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__byte *id)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_simple_iterator_get_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga1b7374cafd886ceb880b050dfa1e387a</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_set_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gae1dd863561606658f88c492682de7b80</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__StreamMetadata *block, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_insert_block_after</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga7a0c00e93bb37324a20926e92e604102</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__StreamMetadata *block, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_delete_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gac3116c8e6e7f59914ae22c0c4c6b0a23</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_Chain *</type>
      <name>FLAC__metadata_chain_new</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga381a1b6efff8d4e9d793f1dda515bd73</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_delete</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46b6c67f30db2955798dfb5556f63aa3</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_ChainStatus</type>
      <name>FLAC__metadata_chain_status</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga8e74773f8ca2bb2bc0b56a65ca0299f4</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga5a4f2056c30f78af5a79f6b64d5bfdcd</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_ogg</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga3995010aab28a483ad9905669e5c4954</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga595f55b611ed588d4d55a9b2eb9d2add</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_ogg_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaccc2f991722682d3c31d36f51985066c</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_check_if_tempfile_needed</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46602f64d423cfe5d5f8a4155f8a97e2</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46bf9cf7d426078101b9297ba80bb835</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga70532b3705294dc891d8db649a4d4843</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write_with_callbacks_and_tempfile</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga72facaa621e8d798036a4a7da3643e41</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks, FLAC__IOHandle temp_handle, FLAC__IOCallbacks temp_callbacks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_merge_padding</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga0a43897914edb751cb87f7e281aff3dc</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_sort_padding</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga82b66fe71c727adb9cf80a1da9834ce5</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_Iterator *</type>
      <name>FLAC__metadata_iterator_new</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga1941ca04671813fc039ea7fd35ae6461</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_iterator_delete</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga374c246e1aeafd803d29a6e99b226241</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_iterator_init</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga2e93196b17a1c73e949e661e33d7311a</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_next</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga60449d0c1d76a73978159e3aa5e79459</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_prev</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaa28df1c5aa56726f573f90e4bae2fe50</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__MetadataType</type>
      <name>FLAC__metadata_iterator_get_block_type</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga83ecb59ffa16bfbb1e286e64f9270de1</anchor>
      <arglist>(const FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_iterator_get_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gad3e7fbc3b3d9c192a3ac425c7b263641</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_set_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaf61795b21300a2b0c9940c11974aab53</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_delete_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gadf860af967d2ee483be01fc0ed8767a9</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__bool replace_with_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_insert_block_before</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga8ac45e2df8b6fd6f5db345c4293aa435</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_insert_block_after</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga55e53757f91696e2578196a2799fc632</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_new</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5df7bc8c72cafed1391bdc5ffc876e0f</anchor>
      <arglist>(FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_clone</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga29af0ecc2a015ef22289f206bc308d80</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_delete</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga6b3159744a1e5c4ce9d349fd0ebae800</anchor>
      <arglist>(FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_is_equal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga6853bcafe731b1db37105d49f3085349</anchor>
      <arglist>(const FLAC__StreamMetadata *block1, const FLAC__StreamMetadata *block2)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_application_set_data</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga11f340e8877c58d231b09841182d66e5</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *data, uint32_t length, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_resize_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga7352bb944c594f447d3ab316244a9895</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_points)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_seektable_set_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac258246fdda91e14110a186c1d8dcc8c</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num, FLAC__StreamMetadata_SeekPoint point)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_insert_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5ba4c8024988af5985877f9e0b3fef38</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num, FLAC__StreamMetadata_SeekPoint point)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_delete_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa138480c7ea602a31109d3870b41a12f</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gacd3e1b83fabc1dabccb725b2876c8f53</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_placeholders</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac509d8cb126d06f4bd73505b6c432338</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0b3aca4fbebc206cd79f13ac36f653f0</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__uint64 sample_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga409f80cb3938814ae307e609faabccc4</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__uint64 sample_numbers[], uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_spaced_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab899d58863aa6e974b3ed4ddd2ebf09e</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t num, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_spaced_points_by_samples</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab91c8b020a1da37d7524051ae82328cb</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t samples, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_sort</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gafb0449b639ba5c618826d893c2961260</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__bool compact)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_set_vendor_string</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5cf1a57afab200b4b67730a77d3ee162</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_resize_comments</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab44132276cbec9abcadbacafbcd5f92a</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_comments)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_set_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0661d2b99c0e37fd8c5aa673eb302c03</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_insert_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga395fcb4900cd5710e67dc96a9a9cca70</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_append_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga889b8b9c5bbd1070a1214c3da8b72863</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_replace_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0608308e8c4c09aa610747d8dff90a34</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool all, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_delete_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac9f51ea4151eb8960e56f31beaa94bd3</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab644c34515c04630c62a7645fab2947e</anchor>
      <arglist>(FLAC__StreamMetadata_VorbisComment_Entry *entry, const char *field_name, const char *field_value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_to_name_value_pair</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga29079764fabda53cb3e890e6d05c8345</anchor>
      <arglist>(const FLAC__StreamMetadata_VorbisComment_Entry entry, char **field_name, char **field_value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_matches</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaad491f6e73bfb7c5a97b75eda7f4392a</anchor>
      <arglist>(const FLAC__StreamMetadata_VorbisComment_Entry entry, const char *field_name, uint32_t field_name_length)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_find_entry_from</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaeaf925bf881fd4e93bf68ce09b935175</anchor>
      <arglist>(const FLAC__StreamMetadata *object, uint32_t offset, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_remove_entry_matching</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga017d743b3200a27b8567ef33592224b8</anchor>
      <arglist>(FLAC__StreamMetadata *object, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_remove_entries_matching</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5a3ff5856098c449622ba850684aec75</anchor>
      <arglist>(FLAC__StreamMetadata *object, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>FLAC__metadata_object_cuesheet_track_new</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gafe2983a9c09685e34626cab39b3fb52c</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>FLAC__metadata_object_cuesheet_track_clone</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga1293d6df6daf2d65143d8bb40eed9261</anchor>
      <arglist>(const FLAC__StreamMetadata_CueSheet_Track *object)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_cuesheet_track_delete</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa533fd7b72fa079e783de4b155b241ce</anchor>
      <arglist>(FLAC__StreamMetadata_CueSheet_Track *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_resize_indices</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga003c90292bc93a877060c34a486fc2b4</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t new_num_indices)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_insert_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga2d66b56b6ebda795ccee86968029e6ad</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num, FLAC__StreamMetadata_CueSheet_Index index)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_insert_blank_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga49ff698f47d914f4e9e45032b3433fba</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_delete_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gabc751423461062096470b31613468feb</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_resize_tracks</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga9c2edc662e4109c0f8ab5fd72bddaccf</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_tracks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_set_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab5f4c6e58c5aa72223e80e7dcdeecfe9</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, FLAC__StreamMetadata_CueSheet_Track *track, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_insert_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa5e7694a181545251f263fcb672abf3d</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, FLAC__StreamMetadata_CueSheet_Track *track, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_insert_blank_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga7ccabeffadad2c13522439f1337718ca</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_delete_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga241f5d623483b5aebc3a721cce3fa8ec</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga1a443d9299ce69694ad59bec4519d7b2</anchor>
      <arglist>(const FLAC__StreamMetadata *object, FLAC__bool check_cd_da_subset, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>FLAC__metadata_object_cuesheet_calculate_cddb_id</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaff2f825950b3e4dda4c8ddbf8e2f7ecd</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_mime_type</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga4511ae9ca994c9f4ab035a3c1aa98f45</anchor>
      <arglist>(FLAC__StreamMetadata *object, char *mime_type, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_description</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga293fe7d8b8b9e49d2414db0925b0f442</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *description, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_data</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga00c330534ef8336ed92b30f9e676bb5f</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *data, FLAC__uint32 length, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga88268a5186e37d4b98b4df7870561128</anchor>
      <arglist>(const FLAC__StreamMetadata *object, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__byte *</type>
      <name>FLAC__metadata_object_get_raw</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga25695e3b6541ed37c94169158cd352f8</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_set_raw</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga2db85cdea68f2fa84dd3f8aa31d9e4eb</anchor>
      <arglist>(FLAC__byte *buffer, FLAC__uint32 length)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__Metadata_SimpleIteratorStatusString</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaa2a8b972800c34f9f5807cadf6ecdb57</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__Metadata_ChainStatusString</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga6498d1976b0d9fa3f8f6295c02e622dd</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>metadata.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC++/</path>
    <filename>FLAC_09_09_2metadata_8h.html</filename>
    <includes id="FLAC_09_09_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="FLAC_2metadata_8h" name="metadata.h" local="yes" imported="no">FLAC/metadata.h</includes>
    <class kind="class">FLAC::Metadata::Prototype</class>
    <class kind="class">FLAC::Metadata::StreamInfo</class>
    <class kind="class">FLAC::Metadata::Padding</class>
    <class kind="class">FLAC::Metadata::Application</class>
    <class kind="class">FLAC::Metadata::SeekTable</class>
    <class kind="class">FLAC::Metadata::VorbisComment</class>
    <class kind="class">FLAC::Metadata::VorbisComment::Entry</class>
    <class kind="class">FLAC::Metadata::CueSheet</class>
    <class kind="class">FLAC::Metadata::CueSheet::Track</class>
    <class kind="class">FLAC::Metadata::Picture</class>
    <class kind="class">FLAC::Metadata::Unknown</class>
    <class kind="class">FLAC::Metadata::SimpleIterator</class>
    <class kind="class">FLAC::Metadata::SimpleIterator::Status</class>
    <class kind="class">FLAC::Metadata::Chain</class>
    <class kind="class">FLAC::Metadata::Chain::Status</class>
    <class kind="class">FLAC::Metadata::Iterator</class>
    <member kind="function">
      <type>Prototype *</type>
      <name>construct_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8df28d7c46448436905e52a01824dbec</anchor>
      <arglist>(::FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>clone</name>
      <anchorfile>group__flacpp__metadata__object.html</anchorfile>
      <anchor>gae18d91726a320349b2c3fb45e79d21fc</anchor>
      <arglist>(const Prototype *)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_streaminfo</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga8fa8da652f33edeb4dabb4ce39fda04b</anchor>
      <arglist>(const char *filename, StreamInfo &amp;streaminfo)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_tags</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga533a71ba745ca03068523a4a45fb0329</anchor>
      <arglist>(const char *filename, VorbisComment *&amp;tags)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_tags</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga85166e6206f3d5635684de4257f2b00e</anchor>
      <arglist>(const char *filename, VorbisComment &amp;tags)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_cuesheet</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga4fad03d91f22d78acf35dd2f35df9ac7</anchor>
      <arglist>(const char *filename, CueSheet *&amp;cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_cuesheet</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaea8f05f89e36af143d73b4280f05cc0e</anchor>
      <arglist>(const char *filename, CueSheet &amp;cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_picture</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaa44df95da4d3abc459fdc526a0d54a55</anchor>
      <arglist>(const char *filename, Picture *&amp;picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_picture</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaa6aea22f1ebeb671db19b73277babdea</anchor>
      <arglist>(const char *filename, Picture &amp;picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>stream_decoder.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>stream__decoder_8h.html</filename>
    <includes id="FLAC_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="format_8h" name="format.h" local="yes" imported="no">format.h</includes>
    <class kind="struct">FLAC__StreamDecoder</class>
    <member kind="typedef">
      <type>FLAC__StreamDecoderReadStatus(*</type>
      <name>FLAC__StreamDecoderReadCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga25d4321dc2f122d35ddc9061f44beae7</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderSeekStatus(*</type>
      <name>FLAC__StreamDecoderSeekCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4c18b0216e0f7a83d7e4e7001230545d</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderTellStatus(*</type>
      <name>FLAC__StreamDecoderTellCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gafdf1852486617a40c285c0d76d451a5a</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderLengthStatus(*</type>
      <name>FLAC__StreamDecoderLengthCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga5363f3b46e3f7d6a73385f6560f7e7ef</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__bool(*</type>
      <name>FLAC__StreamDecoderEofCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4eac094fc609363532d90cf8374b4f7e</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderWriteStatus(*</type>
      <name>FLAC__StreamDecoderWriteCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga61e48dc2c0d2f6c5519290ff046874a4</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamDecoderMetadataCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga6aa87c01744c1c601b7f371f627b6e14</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamDecoderErrorCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac896ee6a12668e9015fab4fbc6aae996</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)</arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderState</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga3adb6891c5871a87cd5bbae6c770ba2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dacf4455f4f681a6737a553e10f614704a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da4c1853ed1babdcede9a908e12cf7ccf7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daccff915757978117720ba1613d088ddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_FRAME</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da06dc6158a51a8eb9537b65f2fbb6dc49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da28ce845052d9d1a780f4107e97f4c853</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da3bc0343f47153c5779baf7f37f6e95cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daf2c6efcabdfe889081c2260e6681db49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ABORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dadb52ab4785bd2eb84a95e8aa82311cd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da0d08c527252420813e6a6d6d3e19324a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da565eaf4d5e68b440ecec771cb22d3427</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderInitStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaaed54a24ac6310d29c5cafba79759c44</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44ac94c7e9396f30642f34805e5d626e011</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8f2188c616c9bc09638eece3ae55f152</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a798ad4b6c4e556fd4cb1afbc29562eca</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a0110567f0715c6f87357388bc7fa98f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8184c306e0cd2565a8c5adc1381cb469</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a98bc501c9b2fb5d92d8bb0b3321d504f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderReadStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad793ead451206c64a91dc0b851027b93</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a9a5be0fcf0279b98b2fd462bc4871d06</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a0a0687d25dc9f7163e6e5e294672170f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a923123aebb349e35662e35a7621b7535</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderSeekStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac8d269e3c7af1a5889d3bd38409ed67d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67daca58132d896ad7755827d3f2b72488cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da969ce92a42a2a95609452e9cf01fcc09</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da4a01f1e48baf015e78535cc20683ec53</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderTellStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga83708207969383bd7b5c1e9148528845</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845a516a202ebf4bb61d4a1fb5b029a104dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845aceefd3feb853d5e68a149f2bdd1a9db1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845add75538234493c9f7a20a846a223ca91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderLengthStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad5860157c2bb34501b8b9370472d727a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aaef01bfcdc3099686e106d8f88397653d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aab000e31c0c20c0d19df4f2203b01ea23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aae35949f46f887e6d826fe0fe4b2a32c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderWriteStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga73f67eb9e0ab57945afe038751bc62c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8acea48326e0ab8370d2814f4126fcb84e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8a23bd6bfec34af704e0d5ea273f14d95d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderErrorStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga130e70bd9a73d3c2416247a3e5132ecf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa3ceec2a553dc142ad487ae88eb6f7222</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfae393a9b91a6b2f23398675b5b57e1e86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa208fe77a04e6ff684e50f0eae1214e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa8b6864ad65edd8fea039838b6d3e5575</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa67ee497c6fe564b50d7a7964ef5cd30a</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoder *</type>
      <name>FLAC__stream_decoder_new</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga529c3c1e46417570767fb8e4c76f5477</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_decoder_delete</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad9cf299956da091111d13e83517d8c44</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_ogg_serial_number</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga7fd232e7a2b5070bd26450487edbc2a1</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, long serial_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_md5_checking</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga8f402243eed54f400ddd2f296ff54497</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad4e685f3d055f70fbaed9ffa4f70f74b</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond_application</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaee1196ff5fa97df9810f708dc2bc8326</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond_all</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga1ce03d8f305a818ff9a573473af99dc4</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad75f067720da89c4e9d96dedc45f73e6</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore_application</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaab41e8bc505b24df4912de53de06b085</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore_all</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaa1307f07fae5d7a4a0c18beeae7ec5e6</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderState</type>
      <name>FLAC__stream_decoder_get_state</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaf99dac2d9255f7db4df8a6d9974a9a9a</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>FLAC__stream_decoder_get_resolved_state_string</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad28257412951ca266751a19e2cf54be2</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_get_md5_checking</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gae27a6b30b55beda03559c12a5df21537</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint64</type>
      <name>FLAC__stream_decoder_get_total_samples</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga930d9b591fcfaea74359c722cdfb980c</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_channels</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga802d5f4c48a711b690d6d66d2e3f20a5</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__ChannelAssignment</type>
      <name>FLAC__stream_decoder_get_channel_assignment</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gae62fdf93c1fedd5fea9258ecdc78bb53</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_bits_per_sample</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga689893cde90c171ca343192e92679842</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_sample_rate</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga95f7cdfefba169d964e3c08672a0f0ad</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_blocksize</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gafe07ad9949cc54944fd369fe9335c4bc</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_get_decode_position</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaffd9b0d0832ed01e6d75930b5391def5</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder, FLAC__uint64 *position)</arglist>
    </member>
    <member kind="function">
      <type>const void *</type>
      <name>FLAC__stream_decoder_get_client_data</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gab14d8d4fa1a66a5a603f96090c2deb07</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga150d381abc5249168e439bc076544b29</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderReadCallback read_callback, FLAC__StreamDecoderSeekCallback seek_callback, FLAC__StreamDecoderTellCallback tell_callback, FLAC__StreamDecoderLengthCallback length_callback, FLAC__StreamDecoderEofCallback eof_callback, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga1b043adeb805c779c1e97cb68959d1ab</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderReadCallback read_callback, FLAC__StreamDecoderSeekCallback seek_callback, FLAC__StreamDecoderTellCallback tell_callback, FLAC__StreamDecoderLengthCallback length_callback, FLAC__StreamDecoderEofCallback eof_callback, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga80aa83631460a53263c84e654586dff0</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FILE *file, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4cc7fbaf905c24d6db48b53b7942fe72</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FILE *file, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_file</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4021ead5cff29fd589c915756f902f1a</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const char *filename, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_file</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga548f15d7724f3bff7f2608abe8b12f6c</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const char *filename, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_finish</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga96c47c96920f363cd0972b54067818a9</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_flush</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga95570a455e582b2ab46ab9bb529f26ac</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_reset</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaa4183c2d925d5a5edddde9d1ca145725</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_single</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga9d6df4a39892c05955122cf7f987f856</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_until_end_of_metadata</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga027ffb5b75dc39b3d26f55c5e6b42682</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_until_end_of_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga89a0723812fa6ef7cdb173715f1bc81f</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_skip_single_frame</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga85b666aba976f29e8dd9d7956fce4301</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_seek_absolute</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga6a2eb6072b9fafefc3f80f1959805ccb</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__uint64 sample)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderStateString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac192360ac435614394bf43235cb7981e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderInitStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga0effa1d3031c3206a1719faf984a4f21</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderReadStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gab1ee941839b05045ae1d73ee0fdcb8c9</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderSeekStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac49aff0593584b7ed5fd0b2508f824fc</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderTellStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga3c1b7d5a174d6c2e6bcf1b9a87b5a5cb</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderLengthStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga792933fa9e8b65bfcac62d82e52415f5</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderWriteStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga9df7f0fd8cf9888f97a52b5f3f33cdb0</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderErrorStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac428c69b084529322df05ee793440b88</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>stream_encoder.h</name>
    <path>/home/martijn/bin/flac-ktmf01/include/FLAC/</path>
    <filename>stream__encoder_8h.html</filename>
    <includes id="FLAC_2export_8h" name="export.h" local="yes" imported="no">export.h</includes>
    <includes id="format_8h" name="format.h" local="yes" imported="no">format.h</includes>
    <includes id="stream__decoder_8h" name="stream_decoder.h" local="yes" imported="no">stream_decoder.h</includes>
    <class kind="struct">FLAC__StreamEncoder</class>
    <member kind="typedef">
      <type>FLAC__StreamEncoderReadStatus(*</type>
      <name>FLAC__StreamEncoderReadCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga18b7941b93bae067192732e913536d44</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__byte buffer[], size_t *bytes, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderWriteStatus(*</type>
      <name>FLAC__StreamEncoderWriteCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2998a0af774d793928a7cc3bbc84dcdf</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderSeekStatus(*</type>
      <name>FLAC__StreamEncoderSeekCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga70b85349d5242e4401c4d8ddf6d9bbca</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderTellStatus(*</type>
      <name>FLAC__StreamEncoderTellCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gabefdf2279e1d0347d9f98f46da4e415b</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamEncoderMetadataCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga091fbf3340d85bcbda1090c31bc320cf</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, const FLAC__StreamMetadata *metadata, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamEncoderProgressCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga42a5fab5f91c1b0c3f7098499285f277</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, uint32_t frames_written, uint32_t total_frames_estimate, void *client_data)</arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderState</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac5e9db4fc32ca2fa74abd9c8a87c02a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a3a6666ae61a64d955341cec285695bf6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a04912e04a3c57d3c53de34742f96d635</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5abb312cc8318c7a541cadacd23ceb3bbb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_DECODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a4cb80be4f83eb71f04e74968af1d259e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_MISMATCH_IN_AUDIO_DATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a011e3d8b2d02a940bfd0e59c05cf5ae0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_CLIENT_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a8c2b2e9efb43a4f9b25b1d2bd9af5f23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_IO_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5af0e4738522e05a7248435c7148f58f91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_FRAMING_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a2c2937b7f1600a4ac7c84fc70ab34cf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a35db99d9958bd6c2301a04715fbc44fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderInitStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3bb869620af2b188d77982a5c30b047d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da20501dce552da74c5df935eeaa0c9ee3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ENCODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da9c64e5f9020d8799e1cd9d39d50e6955</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da8a822b011de88b67c114505ffef39327</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dac2cf461f02e20513003b8cadeae03f9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_NUMBER_OF_CHANNELS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da0541c4f827f081b9f1c54c9441e4aa65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dad6d2631f464183c0c165155200882e6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_SAMPLE_RATE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da6fdcde9e18c37450c79e8f12b9d9c134</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BLOCK_SIZE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da652c445f1bd8b6cfb963a30bf416c95a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_MAX_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da38a69e94b3333e4ba779d2ff8f43f64e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da5be80403bd7a43450139442e0f34ad7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_BLOCK_SIZE_TOO_SMALL_FOR_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da62a17a3ed3c05ddf8ea7f6fecbd4e4a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa793405c858c7606539082750080a47e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_METADATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa85afdd1849c75a19594416cef63e3e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dab4e7b50d176a127575df90383cb15e1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderReadStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2e81f007fb0a7414c0bbb453f37ea37f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa4bdd691d3666f19ec96ff99402347a2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa562fef84bf86a9a39682e23066d9cfee</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa69b94eeab60e07d5fd33f2b3c8b85759</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa9bb730b8f6354cc1e810017a2f700316</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderWriteStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3737471fd49730bb8cf9b182bdeda05e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea5622e0199f0203c402fcb7b4ca76f808</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea18e7cd6a443fb8bd303c3ba89946bc85</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderSeekStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6d5be3489f45fcf0c252022c65d87aca</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa99853066610d798627888ec2e5afa667</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaabf93227938b4e1bf3656fe4ba4159c60</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa8930179a426134caf30a70147448f037</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderTellStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab628f63181250eb977a28bf12b7dd9ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa48e071d89494ac8f5471e7c0d7a6f43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffaf638882e04d7c58e6c29dcc7f410864b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa9d6bbd317f85fd2d6fc72f64e3cb56e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoder *</type>
      <name>FLAC__stream_encoder_new</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab09f7620a0ba9c30020c189ce112a52f</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_encoder_delete</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7212e6846f543618b6289666de216b29</anchor>
      <arglist>(FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_ogg_serial_number</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf4f75f7689b6b3fff16b03028aa38326</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, long serial_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_verify</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga795be6527a9eb1219331afef2f182a41</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_streamable_subset</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga35a18815a58141b88db02317892d059b</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_channels</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9ec612a48f81805eafdb059548cdaf92</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_bits_per_sample</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7453fc29d7e86b499f23b1adfba98da1</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_sample_rate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaa6b6537875900a6e0f4418a504f55f25</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_compression_level</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaacc01aab02849119f929b8516420fcd3</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_blocksize</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac35cb1b5614464658262e684c4ac3a2f</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3bff001a1efc2e4eb520c954066330f4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_loose_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7965d51b93f14cbd6ad5bb9d34f10536</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_apodization</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6598f09ac782a1f2a5743ddf247c81c8</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *specification)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_max_lpc_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gad8a0ff058c46f9ce95dc0508f4bdfb0c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_qlp_coeff_precision</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga179751f915a3d6fc2ca4b33a67bb8780</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_qlp_coeff_prec_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga495890067203958e5d67a641f8757b1c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_escape_coding</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaed594c373d829f77808a935c54a25fa4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_exhaustive_model_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga054313e7f6eaf5c6122d82c6a8b3b808</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_min_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga8f2ed5a2b35bfea13e6605b0fe55f0fa</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_max_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab9e02bfbbb1d4fcdb666e2e9a678b4f6</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_rice_parameter_search_dist</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2cc4a05caba8a4058f744d9eb8732caa</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_total_samples_estimate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab943094585d1c0a4bec497e73567cf85</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__uint64 value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_metadata</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga80d57f9069e354cbf1a15a3e3ad9ca78</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamMetadata **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_limit_min_bitrate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac8c5f361b441d528b7a6791b66bb9d40</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderState</type>
      <name>FLAC__stream_encoder_get_state</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0803321b37189dc5eea4fe1cea25c29a</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderState</type>
      <name>FLAC__stream_encoder_get_verify_decoder_state</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga820704b95a711e77d55363e8753f9f9f</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>FLAC__stream_encoder_get_resolved_state_string</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0916f813358eb6f1e44148353acd4d42</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_encoder_get_verify_decoder_error_stats</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga28373aaf2c47336828d5672696c36662</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_sample, uint32_t *frame_number, uint32_t *channel, uint32_t *sample, FLAC__int32 *expected, FLAC__int32 *got)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_verify</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9efc4964992e001bcec0a8eaedee8d60</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_streamable_subset</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga201e64032ea4298b2379c93652b28245</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_channels</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga412401503141dd42e37831140f78cfa1</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_bits_per_sample</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga169bbf662b2a2df017b93f663deadd1d</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_sample_rate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gae56f27536528f13375ffdd23fa9045f7</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_blocksize</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf8a9715b2d09a6876b8dc104bfd70cdc</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga32da1f89997ab94ce5d677fcd7e24d56</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_loose_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1455859cf3d233bd4dfff86af010f4fa</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_max_lpc_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga5e1d1c9acd3d5a17106b51f0c0107567</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_qlp_coeff_precision</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga909830fb7f4a0a35710452df39c269a3</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_qlp_coeff_prec_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga65bee5a769d4c5fdc95b81c2fb95061c</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_escape_coding</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0c944049800991422c1bfb3b1c0567a5</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_exhaustive_model_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7bc8b32f58df5564db4b6114cb11042d</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_min_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga4fa722297092aeaebc9d9e743a327d14</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_max_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6f5dfbfb5c6e569c4bae5555c9bf87e6</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_rice_parameter_search_dist</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaca0e38f283b2772b92da7cb4495d909a</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint64</type>
      <name>FLAC__stream_encoder_get_total_samples_estimate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaa22d8935bd985b9cccf6592160ffc6f2</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_limit_min_bitrate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga741c26084d203ac24d16c875b5d902ac</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_stream</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7d801879812b48fcbc40f409800c453c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamEncoderWriteCallback write_callback, FLAC__StreamEncoderSeekCallback seek_callback, FLAC__StreamEncoderTellCallback tell_callback, FLAC__StreamEncoderMetadataCallback metadata_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_stream</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9d1981bcd30b8db4d73b5466be5570f5</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamEncoderReadCallback read_callback, FLAC__StreamEncoderWriteCallback write_callback, FLAC__StreamEncoderSeekCallback seek_callback, FLAC__StreamEncoderTellCallback tell_callback, FLAC__StreamEncoderMetadataCallback metadata_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_FILE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga12789a1c4a4e31cd2e7187259fe127f8</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FILE *file, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_FILE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga57fc668f50ffd99a93df326bfab5e2b1</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FILE *file, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_file</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9d5117c2ac0eeb572784116bf2eb541b</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *filename, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_file</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga4891de2f56045941ae222b61b0fd83a4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *filename, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_finish</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3522f9de5af29807df1b9780a418b7f3</anchor>
      <arglist>(FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_process</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga87b9c361292da5c5928a8fb5fda7c423</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const FLAC__int32 *const buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_process_interleaved</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6e31c221f7e23345267c52f53c046c24</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const FLAC__int32 buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderStateString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1410b7a076b0c8401682f9f812b66df5</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderInitStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0ec1fa7b3f55b4f07a2727846c285776</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderReadStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1654422c81846b9b399ac5fb98df61dd</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderWriteStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9f64480accd01525cbfa25c11e6bb74e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderSeekStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gabb137b2d787756bf97398f0b60e54c20</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderTellStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf8ab921ae968be2be255be1f136e1eec</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Decoder::File</name>
    <filename>classFLAC_1_1Decoder_1_1File.html</filename>
    <base>FLAC::Decoder::Stream</base>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a793d2d9c08900cbe6ef6e2739c1e091f</anchor>
      <arglist>(FILE *file)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a4252bc6c949ec9456eea4af2a277dd6a</anchor>
      <arglist>(const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a104a987909937cd716d382fdef9a0245</anchor>
      <arglist>(const std::string &amp;filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>ab840fa309cb000e041f8427cd3e6354a</anchor>
      <arglist>(FILE *file)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a1af59a2861de527e8de5697683516b6e</anchor>
      <arglist>(const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>ac88baae2ff5a4c206a953262cd7447a9</anchor>
      <arglist>(const std::string &amp;filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a33169215b21ff3582c0c1f5fef6dda47</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>adb52518fda2e3e544f4c8807f4227ba7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a031b66dfb0e613a83ac302e7c94c7156</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator bool</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a390efefcf618ca7f3bfcc1d88ecdb4a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_ogg_serial_number</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>aa257e8156474458cd8eed2902d3c2674</anchor>
      <arglist>(long value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_md5_checking</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a8f46d34c10a65d9c48e990f9b3bbe4e2</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a9208dd09a48d7a3034119565f51f0c56</anchor>
      <arglist>(::FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond_application</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a95468ca8d92d1693b21203ad3e0d4545</anchor>
      <arglist>(const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond_all</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a2ecec7b37f6f1d16ddcfee83a6919b5b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ae239124fe0fc8fce3dcdae904bce7544</anchor>
      <arglist>(::FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore_application</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac963b9eaf8271fc47ef799901b6d3650</anchor>
      <arglist>(const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore_all</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a900ecb31410c4ce56f23477b22c1c799</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>State</type>
      <name>get_state</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab9b2544cf4e3b6e045ce3a6341d5a62c</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_md5_checking</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a4264fbd1585cbeb1a28b81c2b09323b6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual FLAC__uint64</type>
      <name>get_total_samples</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac767e144749a6b7f4bb6fa0ab7959114</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_channels</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a599a8cc8fa2522f5886977f616d144d7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__ChannelAssignment</type>
      <name>get_channel_assignment</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a7810225c9440e0bceb4e9c5e8d728be1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a55fa74c9d7a7daf444c43adf624b7a3b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_sample_rate</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a1413d69a409dc80a5774a061915393eb</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_blocksize</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a6f0b833696a9e12c0914f20350af5006</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_decode_position</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a36100b072893e211331099e06084cfab</anchor>
      <arglist>(FLAC__uint64 *position) const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>finish</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0221e9ba254566331e8d0e33579ee3c0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>flush</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a9cb00ff4543d411a9b3c64b1f3f058bb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>reset</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a7b6b4665e139234fa80acd0a1f16ca7c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_single</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab50ff5df74c47f4e0f1c91d63a59f5ac</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_until_end_of_metadata</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab0cabe42278b18e9d3dbfee39cc720cf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_until_end_of_stream</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>afbd6ff20477cae1ace00b8c304a4795a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>skip_single_frame</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a30a738e7ae11f389c58a74f7ff647fe4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>seek_absolute</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac146128003d4ccd46bcffa82003e545c</anchor>
      <arglist>(FLAC__uint64 sample)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderReadStatus</type>
      <name>read_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1File.html</anchorfile>
      <anchor>a48c900fc010f14786e98908377f41195</anchor>
      <arglist>(FLAC__byte buffer[], size_t *bytes)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderSeekStatus</type>
      <name>seek_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>af6f7e0811f34837752fbe20f3348f895</anchor>
      <arglist>(FLAC__uint64 absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderTellStatus</type>
      <name>tell_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0075cb08ab7bf5230ec0360ae3065a50</anchor>
      <arglist>(FLAC__uint64 *absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderLengthStatus</type>
      <name>length_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a6a9af9305783c4af4b93698293dcdf84</anchor>
      <arglist>(FLAC__uint64 *stream_length)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual bool</type>
      <name>eof_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac06aa682efc2e819624e78a3e6b4bd7b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual ::FLAC__StreamDecoderWriteStatus</type>
      <name>write_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>af5a61e9ff720cca3eb38d1f2790f00fb</anchor>
      <arglist>(const ::FLAC__Frame *frame, const FLAC__int32 *const buffer[])=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>metadata_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a20d0873073d9542e08fb48becaa607c9</anchor>
      <arglist>(const ::FLAC__StreamMetadata *metadata)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>error_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0dbadd163ade7bc2d1858e7a435d5e52</anchor>
      <arglist>(::FLAC__StreamDecoderErrorStatus status)=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Decoder::Stream</name>
    <filename>classFLAC_1_1Decoder_1_1Stream.html</filename>
    <class kind="class">FLAC::Decoder::Stream::State</class>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a031b66dfb0e613a83ac302e7c94c7156</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator bool</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a390efefcf618ca7f3bfcc1d88ecdb4a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_ogg_serial_number</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>aa257e8156474458cd8eed2902d3c2674</anchor>
      <arglist>(long value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_md5_checking</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a8f46d34c10a65d9c48e990f9b3bbe4e2</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a9208dd09a48d7a3034119565f51f0c56</anchor>
      <arglist>(::FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond_application</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a95468ca8d92d1693b21203ad3e0d4545</anchor>
      <arglist>(const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_respond_all</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a2ecec7b37f6f1d16ddcfee83a6919b5b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ae239124fe0fc8fce3dcdae904bce7544</anchor>
      <arglist>(::FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore_application</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac963b9eaf8271fc47ef799901b6d3650</anchor>
      <arglist>(const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata_ignore_all</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a900ecb31410c4ce56f23477b22c1c799</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>State</type>
      <name>get_state</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab9b2544cf4e3b6e045ce3a6341d5a62c</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_md5_checking</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a4264fbd1585cbeb1a28b81c2b09323b6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual FLAC__uint64</type>
      <name>get_total_samples</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac767e144749a6b7f4bb6fa0ab7959114</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_channels</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a599a8cc8fa2522f5886977f616d144d7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__ChannelAssignment</type>
      <name>get_channel_assignment</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a7810225c9440e0bceb4e9c5e8d728be1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a55fa74c9d7a7daf444c43adf624b7a3b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_sample_rate</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a1413d69a409dc80a5774a061915393eb</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_blocksize</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a6f0b833696a9e12c0914f20350af5006</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_decode_position</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a36100b072893e211331099e06084cfab</anchor>
      <arglist>(FLAC__uint64 *position) const</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a33169215b21ff3582c0c1f5fef6dda47</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamDecoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>adb52518fda2e3e544f4c8807f4227ba7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>finish</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0221e9ba254566331e8d0e33579ee3c0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>flush</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a9cb00ff4543d411a9b3c64b1f3f058bb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>reset</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a7b6b4665e139234fa80acd0a1f16ca7c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_single</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab50ff5df74c47f4e0f1c91d63a59f5ac</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_until_end_of_metadata</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ab0cabe42278b18e9d3dbfee39cc720cf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_until_end_of_stream</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>afbd6ff20477cae1ace00b8c304a4795a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>skip_single_frame</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a30a738e7ae11f389c58a74f7ff647fe4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>seek_absolute</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac146128003d4ccd46bcffa82003e545c</anchor>
      <arglist>(FLAC__uint64 sample)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual ::FLAC__StreamDecoderReadStatus</type>
      <name>read_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>af91735b6c715ca648493e837f513ef3d</anchor>
      <arglist>(FLAC__byte buffer[], size_t *bytes)=0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderSeekStatus</type>
      <name>seek_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>af6f7e0811f34837752fbe20f3348f895</anchor>
      <arglist>(FLAC__uint64 absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderTellStatus</type>
      <name>tell_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0075cb08ab7bf5230ec0360ae3065a50</anchor>
      <arglist>(FLAC__uint64 *absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamDecoderLengthStatus</type>
      <name>length_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a6a9af9305783c4af4b93698293dcdf84</anchor>
      <arglist>(FLAC__uint64 *stream_length)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual bool</type>
      <name>eof_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>ac06aa682efc2e819624e78a3e6b4bd7b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual ::FLAC__StreamDecoderWriteStatus</type>
      <name>write_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>af5a61e9ff720cca3eb38d1f2790f00fb</anchor>
      <arglist>(const ::FLAC__Frame *frame, const FLAC__int32 *const buffer[])=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>metadata_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a20d0873073d9542e08fb48becaa607c9</anchor>
      <arglist>(const ::FLAC__StreamMetadata *metadata)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>error_callback</name>
      <anchorfile>classFLAC_1_1Decoder_1_1Stream.html</anchorfile>
      <anchor>a0dbadd163ade7bc2d1858e7a435d5e52</anchor>
      <arglist>(::FLAC__StreamDecoderErrorStatus status)=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Decoder::Stream::State</name>
    <filename>classFLAC_1_1Decoder_1_1Stream_1_1State.html</filename>
  </compound>
  <compound kind="class">
    <name>FLAC::Encoder::File</name>
    <filename>classFLAC_1_1Encoder_1_1File.html</filename>
    <base>FLAC::Encoder::Stream</base>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>afefae0d1c92f0d63d7be69a54667ff79</anchor>
      <arglist>(FILE *file)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a31016dd8e1db5bb9c1c3739b94fdb3e3</anchor>
      <arglist>(const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a4966ed5f77dbf5a03946ff25f60a0f8c</anchor>
      <arglist>(const std::string &amp;filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a5dfab60d9cae983899e0b0f6e1ab9377</anchor>
      <arglist>(FILE *file)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a0740ed07b77e49a76f8ddc0e79540eae</anchor>
      <arglist>(const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a202881c81ed146e9a83f7378cf1de2d6</anchor>
      <arglist>(const std::string &amp;filename)</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a17bfdc6402a626db36ee23985ee959b6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a6cd96756d387c89555b4fb36e3323f35</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7115abbe5b89823738e0d95f5fb77d78</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator bool</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a05ed6d063785bf3eac594480661e8132</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_ogg_serial_number</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>adf54d79eb0e6dce071f46be6f2c2d55c</anchor>
      <arglist>(long value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_verify</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85c2296aedf8d4cd2d9f284b1c3205f8</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_streamable_subset</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85d78d5333b05e8a76a1edc9462dbfbc</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_channels</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a6b9175bcf32b465ef5579cf67b23c461</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a6db7416a187b853d612fa060d93fb460</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_sample_rate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5b26c4a46d80d8c5e1711d2f1cac9ff3</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_compression_level</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a19e62dc289edf88ad5ec83f4bb3a4aed</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_blocksize</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a448c7b7bfb8579f78576532fb6db5d9d</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a034ab145e428444b0c6cc4d6818b1121</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_loose_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa691def57681119f0cb99804db7959d0</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_apodization</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4b9a35fd8996be1a4c46fafd41e34e28</anchor>
      <arglist>(const char *specification)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_max_lpc_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aff086f1265804e40504b3a471ffbf1c6</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_qlp_coeff_precision</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a68454d727b7df082b1ca6e20542f0493</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_qlp_coeff_prec_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a9a63c0657c6834229d67e64adaf61fde</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_escape_coding</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4a5b69ec2f0a329a662519021a022266</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_exhaustive_model_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a3832c6e375edfb304ea6dcf7afb15c83</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_min_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4574d815ae9367fc0972ebda437fe27c</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_max_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a0933895f3d004edbd7d5266185c43e28</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_rice_parameter_search_dist</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a859360cccd85c279f3a032b8d578976c</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_total_samples_estimate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5f9de26084c378a7cd55919381465c24</anchor>
      <arglist>(FLAC__uint64 value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac0fe4955fb5e49f4a97cb5bf942c3b03</anchor>
      <arglist>(::FLAC__StreamMetadata **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a66c62377bda60758c7ebf5c5abb8a516</anchor>
      <arglist>(FLAC::Metadata::Prototype **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_limit_min_bitrate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a1f17583a5d4d35b89ce742c0c1bc401d</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function">
      <type>State</type>
      <name>get_state</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa10fe1df856bdf720c598d8512c0b91d</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Decoder::Stream::State</type>
      <name>get_verify_decoder_state</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a8e5bd3b3bcf7bb28ac5bd99045227d71</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>get_verify_decoder_error_stats</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a2016d7cebb7daa740c5751917b922319</anchor>
      <arglist>(FLAC__uint64 *absolute_sample, uint32_t *frame_number, uint32_t *channel, uint32_t *sample, FLAC__int32 *expected, FLAC__int32 *got)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_verify</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa37963386c64655f2472f70d6ef78995</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_streamable_subset</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4cb50455b54a99922bb1c3032ac3c12f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a0174159dde34f8235e0c8ecdf530f655</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_loose_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a71efc8132af5742aa9e243be565c7eda</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_channels</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a98a887884592b75ef7e84421eb0e0d36</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5a3dbd29faf0e10947bc9a52bb686cd5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_sample_rate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac6ac01067586112a448ac0b856c1f722</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_blocksize</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a72f1cb4f655ba38dfbcc5ddff660b34a</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_max_lpc_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab5809af7b04e2fd61116ff9f215568b0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_qlp_coeff_precision</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85b5987212037e8f71dc7d215a31fe9a</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_qlp_coeff_prec_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7a1d05858b28f916ec04c74865da0122</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_escape_coding</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab728524b3c28fa331309c83bea23c0b5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_exhaustive_model_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a14083e5a1b62425335fdb957d6d0e1b9</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_min_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aba92b184c09870ec2bc0e3b06dcb7358</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_max_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a71f704ca4bfd47bffb9d7e295b652b93</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_rice_parameter_search_dist</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab61f5dc890c98a122ae9aa9646d845f4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual FLAC__uint64</type>
      <name>get_total_samples_estimate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>acfb2d26a0546b741fcccd5ede2756072</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_limit_min_bitrate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac70c897d3648ca801dd161a6cae17838</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>finish</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ad70a30287eb9e062454ca296b9628318</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac59f444575b9d745bf6ea7b824e9507f</anchor>
      <arglist>(const FLAC__int32 *const buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_interleaved</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ace0f417b4dff658f6d689a04114d6999</anchor>
      <arglist>(const FLAC__int32 buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>progress_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>ac4c54a7df4723015afeb669131df17bf</anchor>
      <arglist>(FLAC__uint64 bytes_written, FLAC__uint64 samples_written, uint32_t frames_written, uint32_t total_frames_estimate)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderWriteStatus</type>
      <name>write_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1File.html</anchorfile>
      <anchor>a64c0e5118aa2d56f9e671e609728680e</anchor>
      <arglist>(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderReadStatus</type>
      <name>read_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a483965ffe35ed652a5fca622c7791811</anchor>
      <arglist>(FLAC__byte buffer[], size_t *bytes)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderSeekStatus</type>
      <name>seek_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7df3745afe10cd4dbcc3433a32fcb463</anchor>
      <arglist>(FLAC__uint64 absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderTellStatus</type>
      <name>tell_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5a4f38682e33172f53f7f374372fe1e0</anchor>
      <arglist>(FLAC__uint64 *absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>metadata_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ad9c6a7aa7720f215bfe3b65e032e148c</anchor>
      <arglist>(const ::FLAC__StreamMetadata *metadata)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Encoder::Stream</name>
    <filename>classFLAC_1_1Encoder_1_1Stream.html</filename>
    <class kind="class">FLAC::Encoder::Stream::State</class>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7115abbe5b89823738e0d95f5fb77d78</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator bool</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a05ed6d063785bf3eac594480661e8132</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_ogg_serial_number</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>adf54d79eb0e6dce071f46be6f2c2d55c</anchor>
      <arglist>(long value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_verify</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85c2296aedf8d4cd2d9f284b1c3205f8</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_streamable_subset</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85d78d5333b05e8a76a1edc9462dbfbc</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_channels</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a6b9175bcf32b465ef5579cf67b23c461</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a6db7416a187b853d612fa060d93fb460</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_sample_rate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5b26c4a46d80d8c5e1711d2f1cac9ff3</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_compression_level</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a19e62dc289edf88ad5ec83f4bb3a4aed</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_blocksize</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a448c7b7bfb8579f78576532fb6db5d9d</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a034ab145e428444b0c6cc4d6818b1121</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_loose_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa691def57681119f0cb99804db7959d0</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_apodization</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4b9a35fd8996be1a4c46fafd41e34e28</anchor>
      <arglist>(const char *specification)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_max_lpc_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aff086f1265804e40504b3a471ffbf1c6</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_qlp_coeff_precision</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a68454d727b7df082b1ca6e20542f0493</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_qlp_coeff_prec_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a9a63c0657c6834229d67e64adaf61fde</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_escape_coding</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4a5b69ec2f0a329a662519021a022266</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_do_exhaustive_model_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a3832c6e375edfb304ea6dcf7afb15c83</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_min_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4574d815ae9367fc0972ebda437fe27c</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_max_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a0933895f3d004edbd7d5266185c43e28</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_rice_parameter_search_dist</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a859360cccd85c279f3a032b8d578976c</anchor>
      <arglist>(uint32_t value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_total_samples_estimate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5f9de26084c378a7cd55919381465c24</anchor>
      <arglist>(FLAC__uint64 value)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac0fe4955fb5e49f4a97cb5bf942c3b03</anchor>
      <arglist>(::FLAC__StreamMetadata **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_metadata</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a66c62377bda60758c7ebf5c5abb8a516</anchor>
      <arglist>(FLAC::Metadata::Prototype **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>set_limit_min_bitrate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a1f17583a5d4d35b89ce742c0c1bc401d</anchor>
      <arglist>(bool value)</arglist>
    </member>
    <member kind="function">
      <type>State</type>
      <name>get_state</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa10fe1df856bdf720c598d8512c0b91d</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Decoder::Stream::State</type>
      <name>get_verify_decoder_state</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a8e5bd3b3bcf7bb28ac5bd99045227d71</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>get_verify_decoder_error_stats</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a2016d7cebb7daa740c5751917b922319</anchor>
      <arglist>(FLAC__uint64 *absolute_sample, uint32_t *frame_number, uint32_t *channel, uint32_t *sample, FLAC__int32 *expected, FLAC__int32 *got)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_verify</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aa37963386c64655f2472f70d6ef78995</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_streamable_subset</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a4cb50455b54a99922bb1c3032ac3c12f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a0174159dde34f8235e0c8ecdf530f655</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_loose_mid_side_stereo</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a71efc8132af5742aa9e243be565c7eda</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_channels</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a98a887884592b75ef7e84421eb0e0d36</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_bits_per_sample</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5a3dbd29faf0e10947bc9a52bb686cd5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_sample_rate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac6ac01067586112a448ac0b856c1f722</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_blocksize</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a72f1cb4f655ba38dfbcc5ddff660b34a</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_max_lpc_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab5809af7b04e2fd61116ff9f215568b0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_qlp_coeff_precision</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a85b5987212037e8f71dc7d215a31fe9a</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_qlp_coeff_prec_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7a1d05858b28f916ec04c74865da0122</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_escape_coding</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab728524b3c28fa331309c83bea23c0b5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_do_exhaustive_model_search</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a14083e5a1b62425335fdb957d6d0e1b9</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_min_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>aba92b184c09870ec2bc0e3b06dcb7358</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_max_residual_partition_order</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a71f704ca4bfd47bffb9d7e295b652b93</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual uint32_t</type>
      <name>get_rice_parameter_search_dist</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ab61f5dc890c98a122ae9aa9646d845f4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual FLAC__uint64</type>
      <name>get_total_samples_estimate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>acfb2d26a0546b741fcccd5ede2756072</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>get_limit_min_bitrate</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac70c897d3648ca801dd161a6cae17838</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a17bfdc6402a626db36ee23985ee959b6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>virtual ::FLAC__StreamEncoderInitStatus</type>
      <name>init_ogg</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a6cd96756d387c89555b4fb36e3323f35</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>finish</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ad70a30287eb9e062454ca296b9628318</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ac59f444575b9d745bf6ea7b824e9507f</anchor>
      <arglist>(const FLAC__int32 *const buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>process_interleaved</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ace0f417b4dff658f6d689a04114d6999</anchor>
      <arglist>(const FLAC__int32 buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderReadStatus</type>
      <name>read_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a483965ffe35ed652a5fca622c7791811</anchor>
      <arglist>(FLAC__byte buffer[], size_t *bytes)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual ::FLAC__StreamEncoderWriteStatus</type>
      <name>write_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ad225a9143e538103fa88865c3750ad8b</anchor>
      <arglist>(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)=0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderSeekStatus</type>
      <name>seek_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a7df3745afe10cd4dbcc3433a32fcb463</anchor>
      <arglist>(FLAC__uint64 absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>virtual ::FLAC__StreamEncoderTellStatus</type>
      <name>tell_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>a5a4f38682e33172f53f7f374372fe1e0</anchor>
      <arglist>(FLAC__uint64 *absolute_byte_offset)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>metadata_callback</name>
      <anchorfile>classFLAC_1_1Encoder_1_1Stream.html</anchorfile>
      <anchor>ad9c6a7aa7720f215bfe3b65e032e148c</anchor>
      <arglist>(const ::FLAC__StreamMetadata *metadata)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Encoder::Stream::State</name>
    <filename>classFLAC_1_1Encoder_1_1Stream_1_1State.html</filename>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Application</name>
    <filename>classFLAC_1_1Metadata_1_1Application.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>Application</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac852c4aa3be004f1ffa4895ca54354a0</anchor>
      <arglist>(const Application &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Application</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga354471e537af33ba0c86de4db988efd1</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Application &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3ca9dd06666b1dc7d4bdb6aef8e14d04</anchor>
      <arglist>(const Application &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Application &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga47f68d7001ef094a916d3b13fe589fc2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga89c2e1e78226550b47fceb2ab7fe1fa8</anchor>
      <arglist>(const Application &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadf4f2c38053d0d39e735c5f30b9934cf</anchor>
      <arglist>(const Application &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga95eaa06ca65af25385cf05f4942100b8</anchor>
      <arglist>(const FLAC__byte *data, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Chain</name>
    <filename>classFLAC_1_1Metadata_1_1Chain.html</filename>
    <class kind="class">FLAC::Metadata::Chain::Status</class>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga62ff055714c8ce75d907ae58738113a4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Status</type>
      <name>status</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga02d7a4adc89e37b28eaccbccfe5da5b0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>read</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga509bf6a75a12df65bc77947a4765d9c1</anchor>
      <arglist>(const char *filename, bool is_ogg=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>read</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga030c805328fc8b2da947830959dafb5b</anchor>
      <arglist>(FLAC__IOHandle handle, FLAC__IOCallbacks callbacks, bool is_ogg=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>check_if_tempfile_needed</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d54ed419365faf5429caa84b35265c3</anchor>
      <arglist>(bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2341690885e2312013afc561e6fafd81</anchor>
      <arglist>(bool use_padding=true, bool preserve_file_stats=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0ef47e1634bca2d269ac49fc164306b5</anchor>
      <arglist>(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga37b863c4d490fea96f67294f03fbe975</anchor>
      <arglist>(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks, ::FLAC__IOHandle temp_handle, ::FLAC__IOCallbacks temp_callbacks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>merge_padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaef51a0414284f468a2d73c07b540641d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sort_padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga779eaac12da7e7edac67089053e5907f</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Chain::Status</name>
    <filename>classFLAC_1_1Metadata_1_1Chain_1_1Status.html</filename>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::CueSheet</name>
    <filename>classFLAC_1_1Metadata_1_1CueSheet.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <class kind="class">FLAC::Metadata::CueSheet::Track</class>
    <member kind="function">
      <type></type>
      <name>CueSheet</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaff87fa8ab761fc12c0f37b6ff033f74e</anchor>
      <arglist>(const CueSheet &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>CueSheet</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadd934e1916c2427197f8a5654f7ffae9</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>CueSheet &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad24bf2e19de81159d5e205ae5ef63843</anchor>
      <arglist>(const CueSheet &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>CueSheet &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac83a472ca9852f3e2e800ae57d3e1305</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad101b9f069c4af9053718b408a9737f5</anchor>
      <arglist>(const CueSheet &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad02b4b1f541c8607a233a248ec295db9</anchor>
      <arglist>(const CueSheet &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_indices</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7dd7822a201fa2310410029a36f4f1ac</anchor>
      <arglist>(uint32_t track_num, uint32_t new_num_indices)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacebb3ac32324137091b965a9e9ba2edf</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num, const ::FLAC__StreamMetadata_CueSheet_Index &amp;index)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_blank_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga294125ebcaf6c1576759b74f4ba96aa6</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga01c9f6ec36ba9b538ac3c9de993551f8</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_tracks</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8d574ef586ab17413dbf1cb45b630a69</anchor>
      <arglist>(uint32_t new_num_tracks)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5854e1797bf5161d1dc7e9cca5201bc9</anchor>
      <arglist>(uint32_t i, const Track &amp;track)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaeef4dc2ff2f9cc102855aec900860ce6</anchor>
      <arglist>(uint32_t i, const Track &amp;track)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_blank_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gabe22447cc77d2f12092b68493ad2fca5</anchor>
      <arglist>(uint32_t i)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga742ea19be39cd5ad23aeac04671c44ae</anchor>
      <arglist>(uint32_t i)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga920da7efb6143683543440c2409b3d26</anchor>
      <arglist>(bool check_cd_da_subset=false, const char **violation=0) const</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>calculate_cddb_id</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7f03abfc2473e54a766c888c8cd431b6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::CueSheet::Track</name>
    <filename>classFLAC_1_1Metadata_1_1CueSheet_1_1Track.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac0fb597614c2327157e765ea278b014f</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Iterator</name>
    <filename>classFLAC_1_1Metadata_1_1Iterator.html</filename>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga42057c663e277d83cc91763730d38b0f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab5713af7318f10a46bd8b26ce586947c</anchor>
      <arglist>(Chain &amp;chain)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>next</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d2871fc1fdcc5dffee1eafd7019f4a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>prev</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gade6ee6b67b22115959e2adfc65d5d3b4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_block_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa25cb3c27e4d6250f98605f89b0fa904</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>get_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3693233f592b9cb333c437413c6be2a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3123daf89fca2a8981c9f361f466a418</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67adaa4ae39cf405ee0f4674ca8836dd</anchor>
      <arglist>(bool replace_with_padding)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_before</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga86de6d0b21ac08b74a2ea8c1a9adce36</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_after</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga73e7a3f7192f369cb3a19d078da504ab</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Padding</name>
    <filename>classFLAC_1_1Metadata_1_1Padding.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3a5665a824530dec2906d76e665573ee</anchor>
      <arglist>(const Padding &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga358085e3cec897ed0b0c88c8ac04618d</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga86b26d7f7df2a1b3ee0215f2b9352274</anchor>
      <arglist>(uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>Padding &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaece6ab03932bea3f0c32ff3cd88f2617</anchor>
      <arglist>(const Padding &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Padding &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3b7508e56df71854ff1f5ad9570b5684</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1c400bb08e873eae7a1a8640a97d4cde</anchor>
      <arglist>(const Padding &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga12654720889aec7a4694c97f2b1f75b7</anchor>
      <arglist>(const Padding &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga07dae9d71b724f27f4bfbea26d7ab8fc</anchor>
      <arglist>(uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Picture</name>
    <filename>classFLAC_1_1Metadata_1_1Picture.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>Picture</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga368985afb060fe1024129ed808392183</anchor>
      <arglist>(const Picture &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Picture</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga703d5d8a88e9764714ee2dd25806e381</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Picture &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2ab3ef473f6c70aafe5bd3229f397a93</anchor>
      <arglist>(const Picture &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Picture &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa3d7384cb724a842c3471a9ab19f81ed</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1cc03a87e1ada7b81af2dfe487d86fa7</anchor>
      <arglist>(const Picture &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3b0c4fa11c7c54427e7aa690c8998692</anchor>
      <arglist>(const Picture &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>get_colors</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab44cabf75add1973ebde9f5f7ed6b780</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_mime_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gafb4e53cb8ae62ea0d9ebd1afdca40c3f</anchor>
      <arglist>(const char *string)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_description</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1bbcd96802a16fc36ac1b6610cd7d4a3</anchor>
      <arglist>(const FLAC__byte *string)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_colors</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8e7dc667ccc55e60abe2b8a751656097</anchor>
      <arglist>(FLAC__uint32 value) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga301630d1c8f7647d0f192e6a2a03e6ba</anchor>
      <arglist>(const FLAC__byte *data, FLAC__uint32 data_length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf11147e2041b46d679b077e6ac26bea0</anchor>
      <arglist>(const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Prototype</name>
    <filename>classFLAC_1_1Metadata_1_1Prototype.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga698fa1529af534ab5d1d98d0979844f6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gae49fa399a6273ccad7cb0e6f787a3f5c</anchor>
      <arglist>(const Prototype &amp;)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga23ec8d118119578adb95de42fcbbaca2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaea76819568855c4f49f2a23d42a642f2</anchor>
      <arglist>(const Prototype &amp;)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::SeekTable</name>
    <filename>classFLAC_1_1Metadata_1_1SeekTable.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>SeekTable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7f93d054937829a85108cd423a56299f</anchor>
      <arglist>(const SeekTable &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SeekTable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaccd82ef77dcc489280c0f46e443b16c7</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>SeekTable &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac1094c0536952a569e41ba619f9b4ff5</anchor>
      <arglist>(const SeekTable &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>SeekTable &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad9d0036938d6ad1c81180cf1e156b844</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga15966f2e33461ce14c3d98a41d47f94d</anchor>
      <arglist>(const SeekTable &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9b25b057f2fdbdc88e2db66d94ad0de4</anchor>
      <arglist>(const SeekTable &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga09783f913385728901ff93686456d647</anchor>
      <arglist>(uint32_t new_num_points)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad01b009dc3aecd5e881b7b425439643f</anchor>
      <arglist>(uint32_t index, const ::FLAC__StreamMetadata_SeekPoint &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gabc1476cf5960660fa5c5d4a65db1441f</anchor>
      <arglist>(uint32_t index, const ::FLAC__StreamMetadata_SeekPoint &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0d8260db8b7534cc66fe2b80380c91bd</anchor>
      <arglist>(uint32_t index)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8a47e1f8b8331024c2ae977d8bd104d6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_placeholders</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gae8e334f73f3d8870df2e948aa5de1234</anchor>
      <arglist>(uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9c05d6c010988cf2f336ab1c02c3c618</anchor>
      <arglist>(FLAC__uint64 sample_number)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad3c644c5e7de6b944feee725d396b27e</anchor>
      <arglist>(FLAC__uint64 sample_numbers[], uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_spaced_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga6ecfcb2478134b483790276b22a4f8b2</anchor>
      <arglist>(uint32_t num, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_spaced_points_by_samples</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga64bc1300d59e79f6c99356bf4a256383</anchor>
      <arglist>(uint32_t samples, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_sort</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga09cc5c101fc9c26655de9ec91dcb502f</anchor>
      <arglist>(bool compact)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::SimpleIterator</name>
    <filename>classFLAC_1_1Metadata_1_1SimpleIterator.html</filename>
    <class kind="class">FLAC::Metadata::SimpleIterator::Status</class>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d528419f9c71d92b71d1d79cff52207</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>init</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67dc75f18d282f41696467f1fbf5c3e8</anchor>
      <arglist>(const char *filename, bool read_only, bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>Status</type>
      <name>status</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9e681b6ad35b10633002ecea5cab37c3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_writable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga70d7bb568dc6190f9cc5be089eaed03b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>next</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab399f6b8c5e35a1d18588279613ea63c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>prev</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga75a859af156322f451045418876eb6a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac83c8401b2e58a3e4ce03a9996523c44</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>off_t</type>
      <name>get_block_offset</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad3779538af5b3fe7cdd2188c79bc80b0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_block_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga30dff6debdbc72aceac7a69b9c3bea75</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_block_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7e53cef599f3ff984a847a4a251afea5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_application_id</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga426d06a9d079f74e82eaa217f14997a5</anchor>
      <arglist>(FLAC__byte *id)</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>get_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab206e5d7145d3726335d336cbc452598</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0ebd4df55346cbcec9ace04f7d7b484d</anchor>
      <arglist>(Prototype *block, bool use_padding=true)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_after</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d0e512147967b7e12ac22914fbe3818</anchor>
      <arglist>(Prototype *block, bool use_padding=true)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67824deff81e2f49c2f51db6b71565e8</anchor>
      <arglist>(bool use_padding=true)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::SimpleIterator::Status</name>
    <filename>classFLAC_1_1Metadata_1_1SimpleIterator_1_1Status.html</filename>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::StreamInfo</name>
    <filename>classFLAC_1_1Metadata_1_1StreamInfo.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>StreamInfo</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab86611073f13dd3e7aea386bb6f1a7a4</anchor>
      <arglist>(const StreamInfo &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>StreamInfo</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaaf4d96124e2b323398f7edf1aaf28003</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>StreamInfo &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga353a63aa812f125fedec844142946142</anchor>
      <arglist>(const StreamInfo &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>StreamInfo &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad1193a408a5735845dea17a131b7282c</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga4010b479ff46aad5ddd363bf456fbfa1</anchor>
      <arglist>(const StreamInfo &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf0f86d918ae7416e4de77215df6e861b</anchor>
      <arglist>(const StreamInfo &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_min_blocksize</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa53c8d9f0c5c396a51bbf543093121cc</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::Unknown</name>
    <filename>classFLAC_1_1Metadata_1_1Unknown.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <member kind="function">
      <type></type>
      <name>Unknown</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga686a799c353cf7a3dc95bb8899318a6b</anchor>
      <arglist>(const Unknown &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Unknown</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2fb76f94e891c3eea7209a461cab4279</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Unknown &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga295f824df8ed10c3386df72272fdca47</anchor>
      <arglist>(const Unknown &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Unknown &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga4dc5e794c8d529245888414b2bf7d404</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3a94274ea08f3ff252216b82c07b73e1</anchor>
      <arglist>(const Unknown &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa700239bfb0acd74e7e8ca0b1cdfcdb5</anchor>
      <arglist>(const Unknown &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad9618a004195b86f5989f5f0d396d028</anchor>
      <arglist>(const FLAC__byte *data, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::VorbisComment</name>
    <filename>classFLAC_1_1Metadata_1_1VorbisComment.html</filename>
    <base>FLAC::Metadata::Prototype</base>
    <class kind="class">FLAC::Metadata::VorbisComment::Entry</class>
    <member kind="function">
      <type></type>
      <name>VorbisComment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga436a5c6a42a83a88206376805743fe3b</anchor>
      <arglist>(const VorbisComment &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>VorbisComment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga65a73f4665db16ac7aec76e9f5e699f2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>VorbisComment &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga135650367ce6c2c5ce12b534307f1cca</anchor>
      <arglist>(const VorbisComment &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>VorbisComment &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9db2171c398cd62a5907e625c3a6228d</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga40e48312009df9d321a46df47fceb63b</anchor>
      <arglist>(const VorbisComment &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac882ee4619675b1231d38a58af5fc8a8</anchor>
      <arglist>(const VorbisComment &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_vendor_string</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad8cffdb4c43ba01eaa9a3f7be0d5926a</anchor>
      <arglist>(const FLAC__byte *string)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_comments</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad924744735bfd0dad8a30aabe2865cbb</anchor>
      <arglist>(uint32_t new_num_comments)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad179979211b6f4ed4ca0e8df0760b343</anchor>
      <arglist>(uint32_t index, const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab1de71f1c0acdc93c1ed39b6b5e09956</anchor>
      <arglist>(uint32_t index, const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>append_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1126c7a0f25a2cf78efc8317d3a861f2</anchor>
      <arglist>(const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>replace_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga240eb83264d05d953395e75e18e15ee2</anchor>
      <arglist>(const Entry &amp;entry, bool all)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf79834672ef87d30faa4574755f05ef8</anchor>
      <arglist>(uint32_t index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_entry_from</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1a8d3eec60ce932566ce847fb7fbb97d</anchor>
      <arglist>(uint32_t offset, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>remove_entry_matching</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf0770518f35fe18fb9a0cc5c0542c4b7</anchor>
      <arglist>(const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>remove_entries_matching</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadde2dc584e31f29d67fcc6d15d2d1034</anchor>
      <arglist>(const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FLAC::Metadata::VorbisComment::Entry</name>
    <filename>classFLAC_1_1Metadata_1_1VorbisComment_1_1Entry.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga75772bb6b5bf90da459e7fb247239b27</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__EntropyCodingMethod</name>
    <filename>structFLAC____EntropyCodingMethod.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__EntropyCodingMethod_PartitionedRice</name>
    <filename>structFLAC____EntropyCodingMethod__PartitionedRice.html</filename>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gade950cdedc8096355882d77a05873586</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__EntropyCodingMethod_PartitionedRiceContents *</type>
      <name>contents</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2fbfa1bd5656bf620c0bb9f8ba77f579</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__EntropyCodingMethod_PartitionedRiceContents</name>
    <filename>structFLAC____EntropyCodingMethod__PartitionedRiceContents.html</filename>
    <member kind="variable">
      <type>uint32_t *</type>
      <name>parameters</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4e372c3649352f965085054f1580ab67</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t *</type>
      <name>raw_bits</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8b1ff7a7f8b8ec51cd0a1dd21a8d06ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>capacity_by_order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga753f44c8d74e17a258026cdeb9aed017</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__Frame</name>
    <filename>structFLAC____Frame.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__FrameFooter</name>
    <filename>structFLAC____FrameFooter.html</filename>
    <member kind="variable">
      <type>FLAC__uint16</type>
      <name>crc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabdd6d64bf281c49c720b97b955d4eee7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__FrameHeader</name>
    <filename>structFLAC____FrameHeader.html</filename>
    <member kind="variable">
      <type>uint32_t</type>
      <name>blocksize</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1898caa360a783bfa799332573b5c735</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>sample_rate</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2f01343180309a48b91d03bcfd58a5cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>channels</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9518ce587ec26d2c1e315edcc99c1e82</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__ChannelAssignment</type>
      <name>channel_assignment</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9a31f752e16da9d690f8d5ff85aed89c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>bits_per_sample</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabd1db9449935817aedeab02d8aedd2fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__FrameNumberType</type>
      <name>number_type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7a62ec09e6f3029297179ef65377265f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>union FLAC__FrameHeader::@3</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga437756a1b78379eb8d825813f4036a51</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint8</type>
      <name>crc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga980438c380697df6f332cb27dc4672c4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__IOCallbacks</name>
    <filename>structFLAC____IOCallbacks.html</filename>
    <member kind="variable">
      <type>FLAC__IOCallback_Read</type>
      <name>read</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga6dd767bc254e31dc47c9a0d218e72190</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Write</type>
      <name>write</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gad64901e5a5710ee4c3c157c75d51ddc0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Seek</type>
      <name>seek</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gaa1a6f4623965a2d9fcc09b92fabaa1ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Tell</type>
      <name>tell</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga8ff0d175a7b3e9318270e305918df827</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Eof</type>
      <name>eof</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga4810838b77667dc02415c854b2103e66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Close</type>
      <name>close</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga8e447ae1999d9da9ebad5417f47223be</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamDecoder</name>
    <filename>structFLAC____StreamDecoder.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamEncoder</name>
    <filename>structFLAC____StreamEncoder.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata</name>
    <filename>structFLAC____StreamMetadata.html</filename>
    <member kind="variable">
      <type>FLAC__MetadataType</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga39fd0655464f2cc7c9c37ae715088aec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__bool</type>
      <name>is_last</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaef40bbf85abe12e035f66f2d54ed316c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>length</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabcdd1a9220a30da08e713c0ae6767c10</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>union FLAC__StreamMetadata::@4</type>
      <name>data</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaaffe9c1f7369b7d52ffc85d1325ce1f4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_Application</name>
    <filename>structFLAC____StreamMetadata__Application.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_CueSheet</name>
    <filename>structFLAC____StreamMetadata__CueSheet.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>media_catalog_number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga776e6057ac7939fba52edecd44ec45bc</anchor>
      <arglist>[129]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>lead_in</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga43fdc0a538ef2c3e0926ee22814baf40</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__bool</type>
      <name>is_cd</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6af66f921aefc6f779fbc0ab6daeab8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>num_tracks</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga08291d25a5574a089746353ff1af844f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>tracks</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5c0c3440b01b773684d56aeb1e424fab</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_CueSheet_Index</name>
    <filename>structFLAC____StreamMetadata__CueSheet__Index.html</filename>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac221421bca83976925e2a41438157bb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga71edc33c19a749f1dfb3d1429e08c77a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_CueSheet_Track</name>
    <filename>structFLAC____StreamMetadata__CueSheet__Track.html</filename>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga40e1c888253a56b6dc4885a44138d1bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga429103d63c44d1861b4dc0762726701a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>isrc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4990c8b13969f4c62683d915ebbf5744</anchor>
      <arglist>[13]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga10b3f2b3b0374601f1bf49fce91ae544</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>pre_emphasis</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad68cbedf46ac71af5c219263fc70719a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>num_indices</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5f1c1d7e3ddc533938b83951c7b3dda5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__StreamMetadata_CueSheet_Index *</type>
      <name>indices</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga14e0692a77b5b6689e208f48369edb90</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_Padding</name>
    <filename>structFLAC____StreamMetadata__Padding.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>dummy</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5214437fcba7d6abdc3b2435dcaa4124</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_Picture</name>
    <filename>structFLAC____StreamMetadata__Picture.html</filename>
    <member kind="variable">
      <type>FLAC__StreamMetadata_Picture_Type</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaddc05a87a1da1ec7dd2301944ff2819c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>mime_type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9b4af2e10b627c0e79abf4cdd79f80e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte *</type>
      <name>description</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5bbfb168b265edfb0b29cfdb71fb413c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>width</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga18dc6cdef9fa6c815450671f631a1e04</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>height</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga76dbd1212d330807cda289660f5ee754</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>depth</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0f2092ddf28a6803e9c8adb7328c1967</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>colors</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf17c1738bab67eba049ee101acfd36f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>data_length</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb893f63a196f70263468770a90580a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte *</type>
      <name>data</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9c71b5d77920e6d3aee6893795c43605</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_SeekPoint</name>
    <filename>structFLAC____StreamMetadata__SeekPoint.html</filename>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>sample_number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga96a62923f1443fd3a5a3498e701e6ecf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>stream_offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6028398e99f937b002618af677d32c9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>frame_samples</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gadd671150e8ba353cd4664dcf874557c4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_SeekTable</name>
    <filename>structFLAC____StreamMetadata__SeekTable.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_StreamInfo</name>
    <filename>structFLAC____StreamMetadata__StreamInfo.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_Unknown</name>
    <filename>structFLAC____StreamMetadata__Unknown.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_VorbisComment</name>
    <filename>structFLAC____StreamMetadata__VorbisComment.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__StreamMetadata_VorbisComment_Entry</name>
    <filename>structFLAC____StreamMetadata__VorbisComment__Entry.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__Subframe</name>
    <filename>structFLAC____Subframe.html</filename>
  </compound>
  <compound kind="struct">
    <name>FLAC__Subframe_Constant</name>
    <filename>structFLAC____Subframe__Constant.html</filename>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>value</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa2f5b8086802007a2a21c208a42259dd</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__Subframe_Fixed</name>
    <filename>structFLAC____Subframe__Fixed.html</filename>
    <member kind="variable">
      <type>FLAC__EntropyCodingMethod</type>
      <name>entropy_coding_method</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0f17f8f756cd2c8acc0262ef14c37088</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga86cd10934697bc18066f19922470e6c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>warmup</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf85ef3bb17392a0ae8f41eeb98fb7856</anchor>
      <arglist>[FLAC__MAX_FIXED_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>residual</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab91be48874aec97177106a4086163188</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__Subframe_LPC</name>
    <filename>structFLAC____Subframe__LPC.html</filename>
    <member kind="variable">
      <type>FLAC__EntropyCodingMethod</type>
      <name>entropy_coding_method</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gadb1401b2f8af05132420145a99f68c6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6307fecaed886af33803e1d39f4f56da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>qlp_coeff_precision</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga51ea4f57973bf99624b6357d9abef6b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>quantization_level</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaedcf1a3e5e62485e7ce250eda1f3e588</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int32</type>
      <name>qlp_coeff</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad0b37ee925e2124a37fe3a513d5410b8</anchor>
      <arglist>[FLAC__MAX_LPC_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>warmup</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad2d508522eed805514803013cf65edd7</anchor>
      <arglist>[FLAC__MAX_LPC_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>residual</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacae4d0d439ea8900c5771eb967aec9bf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>FLAC__Subframe_Verbatim</name>
    <filename>structFLAC____Subframe__Verbatim.html</filename>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>int32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6bdc2b756ad4151110ec9f86b5fca3e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int64 *</type>
      <name>int64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga20ca19c50b671487f5d1da78b07f1b66</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>porting</name>
    <title>Porting Guide for New Versions</title>
    <filename>group__porting.html</filename>
    <subgroup>porting_1_1_2_to_1_1_3</subgroup>
    <subgroup>porting_1_1_3_to_1_1_4</subgroup>
    <subgroup>porting_1_1_4_to_1_2_0</subgroup>
    <subgroup>porting_1_3_4_to_1_4_0</subgroup>
  </compound>
  <compound kind="group">
    <name>porting_1_1_2_to_1_1_3</name>
    <title>Porting from FLAC 1.1.2 to 1.1.3</title>
    <filename>group__porting__1__1__2__to__1__1__3.html</filename>
  </compound>
  <compound kind="group">
    <name>porting_1_1_3_to_1_1_4</name>
    <title>Porting from FLAC 1.1.3 to 1.1.4</title>
    <filename>group__porting__1__1__3__to__1__1__4.html</filename>
  </compound>
  <compound kind="group">
    <name>porting_1_1_4_to_1_2_0</name>
    <title>Porting from FLAC 1.1.4 to 1.2.0</title>
    <filename>group__porting__1__1__4__to__1__2__0.html</filename>
  </compound>
  <compound kind="group">
    <name>porting_1_3_4_to_1_4_0</name>
    <title>Porting from FLAC 1.3.4 to 1.4.0</title>
    <filename>group__porting__1__3__4__to__1__4__0.html</filename>
    <docanchor file="group__porting__1__3__4__to__1__4__0.html" title="Summary">porting_1_3_4_to_1_4_0_summary</docanchor>
    <docanchor file="group__porting__1__3__4__to__1__4__0.html" title="Breaking changes">porting_1_3_4_to_1_4_0_breaking</docanchor>
    <docanchor file="group__porting__1__3__4__to__1__4__0.html" title="Additions">porting_1_3_4_to_1_4_0_additions</docanchor>
  </compound>
  <compound kind="group">
    <name>flac</name>
    <title>FLAC C API</title>
    <filename>group__flac.html</filename>
    <subgroup>flac_callbacks</subgroup>
    <subgroup>flac_export</subgroup>
    <subgroup>flac_format</subgroup>
    <subgroup>flac_metadata</subgroup>
    <subgroup>flac_decoder</subgroup>
    <subgroup>flac_encoder</subgroup>
  </compound>
  <compound kind="group">
    <name>flac_callbacks</name>
    <title>FLAC/callback.h: I/O callback structures</title>
    <filename>group__flac__callbacks.html</filename>
    <class kind="struct">FLAC__IOCallbacks</class>
    <member kind="typedef">
      <type>void *</type>
      <name>FLAC__IOHandle</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga4c329c3168dee6e352384c5e9306260d</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>size_t(*</type>
      <name>FLAC__IOCallback_Read</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga49d95218a6c09b215cd92cc96de71bf9</anchor>
      <arglist>)(void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>size_t(*</type>
      <name>FLAC__IOCallback_Write</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gad991792235879aecae289b56a112e1b8</anchor>
      <arglist>)(const void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Seek</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gab3942bbbd6ae09bcefe7cb3a0060c49c</anchor>
      <arglist>)(FLAC__IOHandle handle, FLAC__int64 offset, int whence)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__int64(*</type>
      <name>FLAC__IOCallback_Tell</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga45314930cabc2e9c04867eae6bca309f</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Eof</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga00ae3b3d373e691908e9539ebf720675</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="typedef">
      <type>int(*</type>
      <name>FLAC__IOCallback_Close</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga0032267fac38220689778833e08f7387</anchor>
      <arglist>)(FLAC__IOHandle handle)</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Read</type>
      <name>read</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga6dd767bc254e31dc47c9a0d218e72190</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Write</type>
      <name>write</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gad64901e5a5710ee4c3c157c75d51ddc0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Seek</type>
      <name>seek</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>gaa1a6f4623965a2d9fcc09b92fabaa1ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Tell</type>
      <name>tell</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga8ff0d175a7b3e9318270e305918df827</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Eof</type>
      <name>eof</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga4810838b77667dc02415c854b2103e66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__IOCallback_Close</type>
      <name>close</name>
      <anchorfile>group__flac__callbacks.html</anchorfile>
      <anchor>ga8e447ae1999d9da9ebad5417f47223be</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_export</name>
    <title>FLAC/export.h: export symbols</title>
    <filename>group__flac__export.html</filename>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga56ca07df8a23310707732b1c0007d6f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_CURRENT</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga31180fe15eea416cd8957cfca1a4c4f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_REVISION</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga811641dd9f8c542d9260240e7fbe8e93</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC_API_VERSION_AGE</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga1add3e09c8dfd57e8c921f299f0bbec1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>FLAC_API_SUPPORTS_OGG_FLAC</name>
      <anchorfile>group__flac__export.html</anchorfile>
      <anchor>ga84ffcb0af1038c60eb3e21fd002093cf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_format</name>
    <title>FLAC/format.h: format components</title>
    <filename>group__flac__format.html</filename>
    <class kind="struct">FLAC__EntropyCodingMethod_PartitionedRiceContents</class>
    <class kind="struct">FLAC__EntropyCodingMethod_PartitionedRice</class>
    <class kind="struct">FLAC__EntropyCodingMethod</class>
    <class kind="struct">FLAC__Subframe_Constant</class>
    <class kind="struct">FLAC__Subframe_Verbatim</class>
    <class kind="struct">FLAC__Subframe_Fixed</class>
    <class kind="struct">FLAC__Subframe_LPC</class>
    <class kind="struct">FLAC__Subframe</class>
    <class kind="struct">FLAC__FrameHeader</class>
    <class kind="struct">FLAC__FrameFooter</class>
    <class kind="struct">FLAC__Frame</class>
    <class kind="struct">FLAC__StreamMetadata_StreamInfo</class>
    <class kind="struct">FLAC__StreamMetadata_Padding</class>
    <class kind="struct">FLAC__StreamMetadata_Application</class>
    <class kind="struct">FLAC__StreamMetadata_SeekPoint</class>
    <class kind="struct">FLAC__StreamMetadata_SeekTable</class>
    <class kind="struct">FLAC__StreamMetadata_VorbisComment_Entry</class>
    <class kind="struct">FLAC__StreamMetadata_VorbisComment</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet_Index</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet_Track</class>
    <class kind="struct">FLAC__StreamMetadata_CueSheet</class>
    <class kind="struct">FLAC__StreamMetadata_Picture</class>
    <class kind="struct">FLAC__StreamMetadata_Unknown</class>
    <class kind="struct">FLAC__StreamMetadata</class>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_METADATA_TYPE_CODE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga626a412545818c2271fa2202c02ff1d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_BLOCK_SIZE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa5a85c2ea434221ce684be3469517003</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_BLOCK_SIZE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaef78bc1b04f721e7b4563381f5514e8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_BLOCK_SIZE_48000HZ</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8f6ba2c28fbfcf52326d115c95b0a751</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_CHANNELS</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga488aa5678a58d08f984f5d39185b763d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga30b0f21abbb2cdfd461fe04b425b5438</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad0156d56751e80241fa349d1e25064a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__REFERENCE_CODEC_MAX_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0fc418d96053d385fd2f56dce8007fbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_SAMPLE_RATE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga99abeef0c05c6bc76eacfa865abbfa70</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_LPC_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga16108d413f524329f338cff6e05f3aff</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_LPC_ORDER_48000HZ</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9791efa78147196820c86a6041d7774d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MIN_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf52033b2950b9396dd92b167b3bbe4db</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6aa38a4bc5b9d96a78253ccb8b08bd1f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_FIXED_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabd0d5d6fe71b337244712b244ae7cb0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__MAX_RICE_PARTITION_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga78a2e97e230b2aa7f99edc94a466f5bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__SUBSET_MAX_RICE_PARTITION_ORDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab19dec1b56de482ccfeb5f9843f60a14</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_SYNC_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae7ddaf298d3ceb83aae6301908675c1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga06dfae7260da40e4c5f8fc4d531b326c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabdf85aa2c9a483378dfe850b85ab93ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLAC__STREAM_METADATA_HEADER_LENGTH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga706a29b8a14902c457783bfd4fd7bab2</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct FLAC__StreamMetadata</type>
      <name>FLAC__StreamMetadata</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga37ced8d328607ea72b2e51c8ef9e2e58</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__EntropyCodingMethodType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga951733d2ea01943514290012cd622d3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa5253f8b8edc61220739f229a299775dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa202960a608ee91f9f11c2575b9ecc5aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__SubframeType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1f431eaf213e74d7747589932d263348</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_CONSTANT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a9bf56d836aeffb11d614e29ea1cdf2a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_VERBATIM</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a8520596ef07d6c8577f07025f137657b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_FIXED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a6b3cce73039a513f9afefdc8e4f664a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_LPC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a31437462c3e4c3a5a214a91eff8cc3af</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__VerbatimSubframeDataType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabf8b5851429eae13f26267bafe7c5d32</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32a9c1ed26317d9c2fe252bc92a4d1c6e4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32aaf4bfde2c07ab557250a2bdc63e7ad6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__ChannelAssignment</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga79855f8525672e37f299bbe02952ef9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca3c554e4c8512c2de31dfd3305f8b31b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca28d41295b20593561dc9934cc977d5cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9cad155b61582140b2b90362005f1a93e2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_MID_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca85c1512c0473b5ede364a9943759a80c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__FrameNumberType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8fe9ebc78386cd2a3d23b7b8e3818e1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca0b9cbf3853f0ae105cf9b5360164f794</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca9220ce93dcc151e5edd5db7e7155b35a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__MetadataType</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac71714ba8ddbbd66d26bb78a427fac01</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_STREAMINFO</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acffa517e969ba6a868dcf10e5da75c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PADDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a6dcb741fc0aef389580f110e88beb896</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_APPLICATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a2b287a22a1ac9440b309127884c8d41b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_SEEKTABLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a5f6323e489be1318f0e3747960ebdd91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_VORBIS_COMMENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01ad013576bc5196b907547739518605520</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_CUESHEET</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a0b3f07ae60609126562cd0233ce00a65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PICTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf28ae2788366617c1aeab81d5961c6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_UNDEFINED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf6ac61fcc866608f5583c275dc34d47</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__MAX_METADATA_TYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a1a2f283a3dd9e7b46181d7a114ec5805</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamMetadata_Picture_Type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf6d3e836cee023e0b8d897f1fdc9825d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_OTHER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dadd6d6af32499b1973e48c9e8f13357ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON_STANDARD</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5eca52e5cfcb718f33f5fce9b1021a49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825daaf44b9d5fb75dde6941463e5029aa351</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da3e20b405fd4e835ff3a4465b8bcb7c36</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BACK_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da9ae132f2ee7d3baf35f94a9dc9640f62</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAFLET_PAGE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dad3cb471b7925ae5034d9fd9ecfafb87a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_MEDIA</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac994edc4166107ab5790e49f0b57ffd9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAD_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da1282e252e20553c39907074052960f42</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4cead70f8720f180fc220e6df8d55cce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_CONDUCTOR</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dae01a47af0b0c4d89500b755ebca866ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8515523b4c9ab65ffef7db98bc09ceb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_COMPOSER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5ea1554bc96deb45731bc5897600d1c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LYRICIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da86159eda8969514f5992b3e341103f22</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_RECORDING_LOCATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac96e810cdd81465709b4a3a03289e89c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_RECORDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8cee3bb376ed1044b3a7e20b9c971ff1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_PERFORMANCE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4d4dc6904984370501865988d948de3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_VIDEO_SCREEN_CAPTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da7adc2b194968b51768721de7bda39df9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FISH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dabbf0d7c519ae8ba8cec7d1f165f67b0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ILLUSTRATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da89ba412c9d89c937c28afdab508d047a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da751716a4528a78a8d53f435c816c4917</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_PUBLISHER_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da31d75150a4079482fe122e703eff9141</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa5253f8b8edc61220739f229a299775dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga951733d2ea01943514290012cd622d3aa202960a608ee91f9f11c2575b9ecc5aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_CONSTANT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a9bf56d836aeffb11d614e29ea1cdf2a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_VERBATIM</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a8520596ef07d6c8577f07025f137657b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_FIXED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a6b3cce73039a513f9afefdc8e4f664a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__SUBFRAME_TYPE_LPC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga1f431eaf213e74d7747589932d263348a31437462c3e4c3a5a214a91eff8cc3af</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32a9c1ed26317d9c2fe252bc92a4d1c6e4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggabf8b5851429eae13f26267bafe7c5d32aaf4bfde2c07ab557250a2bdc63e7ad6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca3c554e4c8512c2de31dfd3305f8b31b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca28d41295b20593561dc9934cc977d5cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9cad155b61582140b2b90362005f1a93e2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__CHANNEL_ASSIGNMENT_MID_SIDE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga79855f8525672e37f299bbe02952ef9ca85c1512c0473b5ede364a9943759a80c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca0b9cbf3853f0ae105cf9b5360164f794</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gga8fe9ebc78386cd2a3d23b7b8e3818e1ca9220ce93dcc151e5edd5db7e7155b35a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_STREAMINFO</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acffa517e969ba6a868dcf10e5da75c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PADDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a6dcb741fc0aef389580f110e88beb896</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_APPLICATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a2b287a22a1ac9440b309127884c8d41b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_SEEKTABLE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a5f6323e489be1318f0e3747960ebdd91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_VORBIS_COMMENT</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01ad013576bc5196b907547739518605520</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_CUESHEET</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a0b3f07ae60609126562cd0233ce00a65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_PICTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf28ae2788366617c1aeab81d5961c6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_TYPE_UNDEFINED</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01acf6ac61fcc866608f5583c275dc34d47</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__MAX_METADATA_TYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggac71714ba8ddbbd66d26bb78a427fac01a1a2f283a3dd9e7b46181d7a114ec5805</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_OTHER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dadd6d6af32499b1973e48c9e8f13357ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON_STANDARD</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5eca52e5cfcb718f33f5fce9b1021a49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825daaf44b9d5fb75dde6941463e5029aa351</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da3e20b405fd4e835ff3a4465b8bcb7c36</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BACK_COVER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da9ae132f2ee7d3baf35f94a9dc9640f62</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAFLET_PAGE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dad3cb471b7925ae5034d9fd9ecfafb87a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_MEDIA</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac994edc4166107ab5790e49f0b57ffd9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEAD_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da1282e252e20553c39907074052960f42</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ARTIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4cead70f8720f180fc220e6df8d55cce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_CONDUCTOR</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dae01a47af0b0c4d89500b755ebca866ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8515523b4c9ab65ffef7db98bc09ceb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_COMPOSER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da5ea1554bc96deb45731bc5897600d1c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LYRICIST</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da86159eda8969514f5992b3e341103f22</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_RECORDING_LOCATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dac96e810cdd81465709b4a3a03289e89c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_RECORDING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da8cee3bb376ed1044b3a7e20b9c971ff1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_PERFORMANCE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da4d4dc6904984370501865988d948de3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_VIDEO_SCREEN_CAPTURE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da7adc2b194968b51768721de7bda39df9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_FISH</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825dabbf0d7c519ae8ba8cec7d1f165f67b0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_ILLUSTRATION</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da89ba412c9d89c937c28afdab508d047a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_BAND_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da751716a4528a78a8d53f435c816c4917</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_PUBLISHER_LOGOTYPE</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ggaf6d3e836cee023e0b8d897f1fdc9825da31d75150a4079482fe122e703eff9141</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_sample_rate_is_valid</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga48100669b8e8613f1e226c3925f701a8</anchor>
      <arglist>(uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_blocksize_is_subset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4e71651ff9b90b50480f86050d78c16b</anchor>
      <arglist>(uint32_t blocksize, uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_sample_rate_is_subset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae048df385980088b4c29c52aa7207306</anchor>
      <arglist>(uint32_t sample_rate)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_name_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gae5fb55cd5977ebf178c5b38da831c057</anchor>
      <arglist>(const char *name)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_value_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1a5061a12c836cc2ff3967088afda1c4</anchor>
      <arglist>(const FLAC__byte *value, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_vorbiscomment_entry_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1439057dbc3f0719309620caaf82c1b1</anchor>
      <arglist>(const FLAC__byte *entry, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_seektable_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga02ed0843553fb8f718fe8e7c54d12244</anchor>
      <arglist>(const FLAC__StreamMetadata_SeekTable *seek_table)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__format_seektable_sort</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2285adb37d91c41b1f9a5c3b1b35e886</anchor>
      <arglist>(FLAC__StreamMetadata_SeekTable *seek_table)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_cuesheet_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa9ed0fa4ed04dbfdaa163d0f5308c080</anchor>
      <arglist>(const FLAC__StreamMetadata_CueSheet *cue_sheet, FLAC__bool check_cd_da_subset, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__format_picture_is_legal</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga82ca3ffc97c106c61882134f1a7fb1be</anchor>
      <arglist>(const FLAC__StreamMetadata_Picture *picture, const char **violation)</arglist>
    </member>
    <member kind="variable">
      <type>const char *</type>
      <name>FLAC__VERSION_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga52e2616f9a0b94881cd7711c18d62a35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *</type>
      <name>FLAC__VENDOR_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad5cccab0de3adda58914edf3c31fd64f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__byte</type>
      <name>FLAC__STREAM_SYNC_STRING</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3f275a3a6056e0d53df3b72b03adde4b</anchor>
      <arglist>[4]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_SYNC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf836406a1f4c1b37ef6e4023f65c127f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_SYNC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa95eb3cb07b7d503de94521a155af6bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__EntropyCodingMethodTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga41603ac35eed8c77c2f2e0b12067d88a</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t *</type>
      <name>parameters</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4e372c3649352f965085054f1580ab67</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t *</type>
      <name>raw_bits</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8b1ff7a7f8b8ec51cd0a1dd21a8d06ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>capacity_by_order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga753f44c8d74e17a258026cdeb9aed017</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gade950cdedc8096355882d77a05873586</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__EntropyCodingMethod_PartitionedRiceContents *</type>
      <name>contents</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2fbfa1bd5656bf620c0bb9f8ba77f579</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga12fe0569d6d11d6e6ba8d3342196ccc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0c00e7f349eabc3d25dab7223cc5af15</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2_PARAMETER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6d5cfd610e45402ac02d5786bda8a755</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7aed9c761b806bfd787c077da0ab9a07</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga80fb6cc2fb05edcea2a7e3ae004096a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2_ESCAPE_PARAMETER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga12e2bed2777e9beb187498ca116bcb0a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__ENTROPY_CODING_METHOD_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga18e9f8910a79bebe138a76a1a923076f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__SubframeTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga78d78f45f123cfbb50cebd61b96097df</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>value</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa2f5b8086802007a2a21c208a42259dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>int32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6bdc2b756ad4151110ec9f86b5fca3e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>int32</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2e692ef67b97800f7a13c0fd035bfc3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int64 *</type>
      <name>int64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga20ca19c50b671487f5d1da78b07f1b66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int64 *</type>
      <name>int64</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9a0fa9100d8d356ca936bfaca3e9d371</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__EntropyCodingMethod</type>
      <name>entropy_coding_method</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0f17f8f756cd2c8acc0262ef14c37088</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga86cd10934697bc18066f19922470e6c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>warmup</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf85ef3bb17392a0ae8f41eeb98fb7856</anchor>
      <arglist>[FLAC__MAX_FIXED_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>residual</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab91be48874aec97177106a4086163188</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__EntropyCodingMethod</type>
      <name>entropy_coding_method</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gadb1401b2f8af05132420145a99f68c6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>order</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6307fecaed886af33803e1d39f4f56da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>qlp_coeff_precision</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga51ea4f57973bf99624b6357d9abef6b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>quantization_level</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaedcf1a3e5e62485e7ce250eda1f3e588</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int32</type>
      <name>qlp_coeff</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad0b37ee925e2124a37fe3a513d5410b8</anchor>
      <arglist>[FLAC__MAX_LPC_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__int64</type>
      <name>warmup</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad2d508522eed805514803013cf65edd7</anchor>
      <arglist>[FLAC__MAX_LPC_ORDER]</arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__int32 *</type>
      <name>residual</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacae4d0d439ea8900c5771eb967aec9bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga303c4e38674249f42ec8735354622463</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga918e00beab5d7826e37b6397520df4c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_ZERO_PAD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8f4ad64ca91dd750a38b5c2d30838fdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga65c51d6c43f33179072d7225768e14a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf2e0e7e4f28e357646ad7e5dfcc90f2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_CONSTANT_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb235be931ef14cee71ad37bc1924667</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_VERBATIM_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga93b8d9b7b76ff5cefa8ce8965a9dca9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_FIXED_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac7884342f77d4f16f1921a0cc7a2d3ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__SUBFRAME_TYPE_LPC_BYTE_ALIGNED_MASK</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5c1baa1525de2749f74c174fad422266</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__ChannelAssignmentString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab1a1d3929a4e5a5aff2c15010742aa21</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__FrameNumberTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga931a0e63c0f2b31fab801e1dd693fa4e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>blocksize</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga1898caa360a783bfa799332573b5c735</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>sample_rate</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2f01343180309a48b91d03bcfd58a5cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>channels</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9518ce587ec26d2c1e315edcc99c1e82</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__ChannelAssignment</type>
      <name>channel_assignment</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9a31f752e16da9d690f8d5ff85aed89c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>bits_per_sample</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabd1db9449935817aedeab02d8aedd2fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__FrameNumberType</type>
      <name>number_type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7a62ec09e6f3029297179ef65377265f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>union FLAC__FrameHeader::@3</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga437756a1b78379eb8d825813f4036a51</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint8</type>
      <name>crc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga980438c380697df6f332cb27dc4672c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SYNC</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7af18147ae3a5bb75136843f6e271a4d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SYNC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab3821624c367fac8d994d0ab43229c13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaed36cf061a5112a72d33b5fdb2941cf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BLOCKING_STRATEGY_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga73711753949d786e168222b2cf9502dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf9b185ee73ab9166498aa087f506c895</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_SAMPLE_RATE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8c686e8933c321c9d386db6a6f0d5f70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_CHANNEL_ASSIGNMENT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8d2909446c32443619b9967188a07fb7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_BITS_PER_SAMPLE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga47f63b74fff6e3396d6203d1022062be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_ZERO_PAD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3d73f3519e9ec387c1cf5d54bdfb022f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_HEADER_CRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac0478a55947c6fb97f53f6a9222a0952</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint16</type>
      <name>crc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabdd6d64bf281c49c720b97b955d4eee7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__FRAME_FOOTER_CRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3e74578ca10d5a2a80766040443665f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__MetadataTypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa9ad23f06a579d1110d61d54c8c999f0</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga08f9ac0cd9e3fe8db67a16c011b1c9f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga60a3c8fc22960cec9adb6e22b866d61c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaab054a54f7725f6fc250321f245e1f9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gafb35eac8504f1903654cb28f924c5c22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaac031487db3e1961cb5d48f0ce5107b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab7c3111fe0e73ac3b323ba881d02a8b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaae73b50a208bc0b9479b56b5be546f69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0d6496e976945999313c9029dba46b2b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_STREAMINFO_MD5SUM_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga651ba492225f315a70286eccd3c3184b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>dummy</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5214437fcba7d6abdc3b2435dcaa4124</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_APPLICATION_ID_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga8040c7fa72cfc55c74e43d620e64a805</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>sample_number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga96a62923f1443fd3a5a3498e701e6ecf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>stream_offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6028398e99f937b002618af677d32c9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>frame_samples</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gadd671150e8ba353cd4664dcf874557c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9e95bd97ef2fa28b1d5bbd3917160f9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaaa177c78a35cdd323845928326274f63</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_FRAME_SAMPLES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga62341e0615038b3eade3c7691f410cca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const FLAC__uint64</type>
      <name>FLAC__STREAM_METADATA_SEEKPOINT_PLACEHOLDER</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad5d58774aea926635e6841c411d60566</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga7ff8c3f4693944031b9ac8ff99093df6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_VORBIS_COMMENT_NUM_COMMENTS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2019f140758b10d086e438e43a257036</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac221421bca83976925e2a41438157bb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga71edc33c19a749f1dfb3d1429e08c77a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gab448a7b0ee7c06c6fa23155d29c37ccb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9d3b4268a36fa8a5d5f8cf2ee704ceb2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_INDEX_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga978b9c0ec4220d22a6bd4aab75fb9949</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>offset</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga40e1c888253a56b6dc4885a44138d1bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga429103d63c44d1861b4dc0762726701a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>isrc</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4990c8b13969f4c62683d915ebbf5744</anchor>
      <arglist>[13]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga10b3f2b3b0374601f1bf49fce91ae544</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>pre_emphasis</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad68cbedf46ac71af5c219263fc70719a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte</type>
      <name>num_indices</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5f1c1d7e3ddc533938b83951c7b3dda5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__StreamMetadata_CueSheet_Index *</type>
      <name>indices</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga14e0692a77b5b6689e208f48369edb90</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_OFFSET_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gad09fd65eb06250d671d05eb8e999cc89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gac4fb0980ac6a409916e4122ba25ae8fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_ISRC_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga76dc2c2ae2385f2ab0752f16f7f9d4c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf7f2927d240eeab1214a88bceb5deae6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_PRE_EMPHASIS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga715d4e09605238e3b40afdbdaf4717b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga06b1d7142a95fa837eff737ee8f825be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_TRACK_NUM_INDICES_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga4b4231131e11b216e34e49d12f210363</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>media_catalog_number</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga776e6057ac7939fba52edecd44ec45bc</anchor>
      <arglist>[129]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint64</type>
      <name>lead_in</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga43fdc0a538ef2c3e0926ee22814baf40</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__bool</type>
      <name>is_cd</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6af66f921aefc6f779fbc0ab6daeab8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>num_tracks</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga08291d25a5574a089746353ff1af844f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>tracks</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5c0c3440b01b773684d56aeb1e424fab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_MEDIA_CATALOG_NUMBER_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaae2030a18d8421dc476ff18c95f773d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_LEAD_IN_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga397890e4c43ca950d2236250d69a92f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_IS_CD_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga285c570708526c7ebcb742c982e5d5fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_RESERVED_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb9458a79b7d214e8758cc5ad4e2b18a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_CUESHEET_NUM_TRACKS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa30d6a1d38397b4851add1bb2a6d145c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamMetadata_Picture_TypeString</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2d27672452696cb97fd39db1cf43486b</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>FLAC__StreamMetadata_Picture_Type</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaddc05a87a1da1ec7dd2301944ff2819c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>mime_type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9b4af2e10b627c0e79abf4cdd79f80e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte *</type>
      <name>description</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5bbfb168b265edfb0b29cfdb71fb413c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>width</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga18dc6cdef9fa6c815450671f631a1e04</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>height</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga76dbd1212d330807cda289660f5ee754</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>depth</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga0f2092ddf28a6803e9c8adb7328c1967</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>colors</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf17c1738bab67eba049ee101acfd36f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__uint32</type>
      <name>data_length</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gacb893f63a196f70263468770a90580a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__byte *</type>
      <name>data</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9c71b5d77920e6d3aee6893795c43605</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga9a91512adcf0f8293c0a8793ce8b246c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_MIME_TYPE_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga5186600f0920191cb61e55b2c7628287</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DESCRIPTION_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga6d71497d949952f8d8b16f482ebcf555</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_WIDTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga2819d0e2a032fd5947a1259e40b5f52a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_HEIGHT_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaf537b699909721adca031b6e3826ce22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DEPTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga553826edf5d175f81f162e3049c386ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_COLORS_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga3f810c75aad1f5a0c9d1d85c56998b5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_PICTURE_DATA_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gafd1dd421206189d123f644ff3717cb12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__MetadataType</type>
      <name>type</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga39fd0655464f2cc7c9c37ae715088aec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FLAC__bool</type>
      <name>is_last</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaef40bbf85abe12e035f66f2d54ed316c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>length</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gabcdd1a9220a30da08e713c0ae6767c10</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>union FLAC__StreamMetadata::@4</type>
      <name>data</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaaffe9c1f7369b7d52ffc85d1325ce1f4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_IS_LAST_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaa51331191b62fb15793b0a35ea8821e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_TYPE_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>gaec6fd2f0de2c3f88b7bb0449d178043c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const uint32_t</type>
      <name>FLAC__STREAM_METADATA_LENGTH_LEN</name>
      <anchorfile>group__flac__format.html</anchorfile>
      <anchor>ga90cbf669f1c3400813ee4ecdd3462ca3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_metadata</name>
    <title>FLAC/metadata.h: metadata interfaces</title>
    <filename>group__flac__metadata.html</filename>
    <subgroup>flac_metadata_level0</subgroup>
  </compound>
  <compound kind="group">
    <name>flac_metadata_level0</name>
    <title>FLAC/metadata.h: metadata level 0 interface</title>
    <filename>group__flac__metadata__level0.html</filename>
    <subgroup>flac_metadata_level1</subgroup>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_streaminfo</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga804b42d9da714199b4b383ce51078d51</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata *streaminfo)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_tags</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga1626af09cd39d4fa37d5b46ebe3790fd</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **tags)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_cuesheet</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>ga0f47949dca514506718276205a4fae0b</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_get_picture</name>
      <anchorfile>group__flac__metadata__level0.html</anchorfile>
      <anchor>gab9f69e48c5a33cacb924d13986bfb852</anchor>
      <arglist>(const char *filename, FLAC__StreamMetadata **picture, FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_metadata_level1</name>
    <title>FLAC/metadata.h: metadata level 1 interface</title>
    <filename>group__flac__metadata__level1.html</filename>
    <subgroup>flac_metadata_level2</subgroup>
    <member kind="typedef">
      <type>struct FLAC__Metadata_SimpleIterator</type>
      <name>FLAC__Metadata_SimpleIterator</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga6accccddbb867dfc2eece9ee3ffecb3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__Metadata_SimpleIteratorStatus</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gac926e7d2773a05066115cac9048bbec9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a33aadd73194c0d7e307d643237e0ddcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a0a3933cb38c8957a8d5c3d1afb4766f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a20e835bbb74b4d039e598617f68d2af6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a7785f77a612be8956fbe7cab73497220</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9af055d8c0c663e72134fe2db8037b6880</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a14c897124887858109200723826f85b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a088df964f0852dd7e19304e920c3ee8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2ad85a32e291d1e918692d68cc22fd40</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9ac2337299c2347ca311caeaa7d71d857c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2e073843fa99419d76a0b210da96ceb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a4f855433038c576da127fc1de9d18f9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9aa8386ed0a20d7e91b0022d203ec3cdec</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a9d821ae65a1c5de619daa88c850906df</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_SimpleIterator *</type>
      <name>FLAC__metadata_simple_iterator_new</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga017ae86f3351888f50feb47026ed2482</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_simple_iterator_delete</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga4619be06f51429fea71e5b98900cec3e</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_SimpleIteratorStatus</type>
      <name>FLAC__metadata_simple_iterator_status</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gae8fd236fe6049c61f7f3b4a6ecbcd240</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_init</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaba8daf276fd7da863a2522ac050125fd</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, const char *filename, FLAC__bool read_only, FLAC__bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_is_writable</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga5150ecd8668c610f79192a2838667790</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_next</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gabb7de0a1067efae353e0792dc6e51905</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_prev</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga6db5313b31120b28e210ae721d6525a8</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_is_last</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga9eb215059840960de69aa84469ba954f</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>off_t</type>
      <name>FLAC__metadata_simple_iterator_get_block_offset</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gade0a61723420daeb4bc226713671c6f0</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__MetadataType</type>
      <name>FLAC__metadata_simple_iterator_get_block_type</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga17b61d17e83432913abf4334d6e0c073</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__metadata_simple_iterator_get_block_length</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaf29b9a7f2e2c762756c1444e55a119fa</anchor>
      <arglist>(const FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_get_application_id</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gad4fea2d7d98d16e75e6d8260f690a5dc</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__byte *id)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_simple_iterator_get_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga1b7374cafd886ceb880b050dfa1e387a</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_set_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gae1dd863561606658f88c492682de7b80</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__StreamMetadata *block, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_insert_block_after</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ga7a0c00e93bb37324a20926e92e604102</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__StreamMetadata *block, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_simple_iterator_delete_block</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gac3116c8e6e7f59914ae22c0c4c6b0a23</anchor>
      <arglist>(FLAC__Metadata_SimpleIterator *iterator, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__Metadata_SimpleIteratorStatusString</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>gaa2a8b972800c34f9f5807cadf6ecdb57</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_metadata_level2</name>
    <title>FLAC/metadata.h: metadata level 2 interface</title>
    <filename>group__flac__metadata__level2.html</filename>
    <subgroup>flac_metadata_object</subgroup>
    <member kind="typedef">
      <type>struct FLAC__Metadata_Chain</type>
      <name>FLAC__Metadata_Chain</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaec6993c60b88f222a52af86f8f47bfdf</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct FLAC__Metadata_Iterator</type>
      <name>FLAC__Metadata_Iterator</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga9f3e135a07cdef7e51597646aa7b89b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__Metadata_ChainStatus</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gafe2a924893b0800b020bea8160fd4531</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a293be942ec54576f2b3c73613af968e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1be9400982f411173af46bf0c3acbdc7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a43d2741a650576052fa3615d8cd64d86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a99748a4b12ed10f9368375cc8deeb143</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ac469c6543ebb117e99064572c16672d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a8efd2c76dc06308eb6eba59e1bc6300b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0525de5fb5d8aeeb4e848e33a8d503c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a5814bc26bcf92143198b8e7f028f43a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a66460c735e4745788b40889329e8489f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af4ecf22bc3e5adf78a9c765f856efb0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1cd3138ed493f6a0f5b95fb8481edd1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab12ec938f7556a163c609194ee0aede0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a36b9bcf93da8e0f111738a65eab36e9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab8a6aa5f115db3f07ad2ed4adbcbe060</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_WRITE_MISMATCH</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0d9e64ad6514c88b8ea9e9171c42ec9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRONG_WRITE_CALL</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af86670707345e2d02cc84aec059459d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_Chain *</type>
      <name>FLAC__metadata_chain_new</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga381a1b6efff8d4e9d793f1dda515bd73</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_delete</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46b6c67f30db2955798dfb5556f63aa3</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_ChainStatus</type>
      <name>FLAC__metadata_chain_status</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga8e74773f8ca2bb2bc0b56a65ca0299f4</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga5a4f2056c30f78af5a79f6b64d5bfdcd</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_ogg</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga3995010aab28a483ad9905669e5c4954</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, const char *filename)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga595f55b611ed588d4d55a9b2eb9d2add</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_read_ogg_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaccc2f991722682d3c31d36f51985066c</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_check_if_tempfile_needed</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46602f64d423cfe5d5f8a4155f8a97e2</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga46bf9cf7d426078101b9297ba80bb835</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write_with_callbacks</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga70532b3705294dc891d8db649a4d4843</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_chain_write_with_callbacks_and_tempfile</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga72facaa621e8d798036a4a7da3643e41</anchor>
      <arglist>(FLAC__Metadata_Chain *chain, FLAC__bool use_padding, FLAC__IOHandle handle, FLAC__IOCallbacks callbacks, FLAC__IOHandle temp_handle, FLAC__IOCallbacks temp_callbacks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_merge_padding</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga0a43897914edb751cb87f7e281aff3dc</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_chain_sort_padding</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga82b66fe71c727adb9cf80a1da9834ce5</anchor>
      <arglist>(FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__Metadata_Iterator *</type>
      <name>FLAC__metadata_iterator_new</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga1941ca04671813fc039ea7fd35ae6461</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_iterator_delete</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga374c246e1aeafd803d29a6e99b226241</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_iterator_init</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga2e93196b17a1c73e949e661e33d7311a</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__Metadata_Chain *chain)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_next</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga60449d0c1d76a73978159e3aa5e79459</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_prev</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaa28df1c5aa56726f573f90e4bae2fe50</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__MetadataType</type>
      <name>FLAC__metadata_iterator_get_block_type</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga83ecb59ffa16bfbb1e286e64f9270de1</anchor>
      <arglist>(const FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_iterator_get_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gad3e7fbc3b3d9c192a3ac425c7b263641</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_set_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gaf61795b21300a2b0c9940c11974aab53</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_delete_block</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>gadf860af967d2ee483be01fc0ed8767a9</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__bool replace_with_padding)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_insert_block_before</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga8ac45e2df8b6fd6f5db345c4293aa435</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_iterator_insert_block_after</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga55e53757f91696e2578196a2799fc632</anchor>
      <arglist>(FLAC__Metadata_Iterator *iterator, FLAC__StreamMetadata *block)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__Metadata_ChainStatusString</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ga6498d1976b0d9fa3f8f6295c02e622dd</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_metadata_object</name>
    <title>FLAC/metadata.h: metadata object methods</title>
    <filename>group__flac__metadata__object.html</filename>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a33aadd73194c0d7e307d643237e0ddcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a0a3933cb38c8957a8d5c3d1afb4766f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a20e835bbb74b4d039e598617f68d2af6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a7785f77a612be8956fbe7cab73497220</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9af055d8c0c663e72134fe2db8037b6880</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a14c897124887858109200723826f85b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a088df964f0852dd7e19304e920c3ee8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2ad85a32e291d1e918692d68cc22fd40</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9ac2337299c2347ca311caeaa7d71d857c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a2e073843fa99419d76a0b210da96ceb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a4f855433038c576da127fc1de9d18f9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9aa8386ed0a20d7e91b0022d203ec3cdec</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_SIMPLE_ITERATOR_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level1.html</anchorfile>
      <anchor>ggac926e7d2773a05066115cac9048bbec9a9d821ae65a1c5de619daa88c850906df</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_OK</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a293be942ec54576f2b3c73613af968e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ILLEGAL_INPUT</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1be9400982f411173af46bf0c3acbdc7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a43d2741a650576052fa3615d8cd64d86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_A_FLAC_FILE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a99748a4b12ed10f9368375cc8deeb143</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_NOT_WRITABLE</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ac469c6543ebb117e99064572c16672d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a8efd2c76dc06308eb6eba59e1bc6300b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0525de5fb5d8aeeb4e848e33a8d503c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_SEEK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a5814bc26bcf92143198b8e7f028f43a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRITE_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a66460c735e4745788b40889329e8489f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_RENAME_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af4ecf22bc3e5adf78a9c765f856efb0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_UNLINK_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a1cd3138ed493f6a0f5b95fb8481edd1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab12ec938f7556a163c609194ee0aede0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INTERNAL_ERROR</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a36b9bcf93da8e0f111738a65eab36e9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531ab8a6aa5f115db3f07ad2ed4adbcbe060</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_READ_WRITE_MISMATCH</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531a0d9e64ad6514c88b8ea9e9171c42ec9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__METADATA_CHAIN_STATUS_WRONG_WRITE_CALL</name>
      <anchorfile>group__flac__metadata__level2.html</anchorfile>
      <anchor>ggafe2a924893b0800b020bea8160fd4531af86670707345e2d02cc84aec059459d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_new</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5df7bc8c72cafed1391bdc5ffc876e0f</anchor>
      <arglist>(FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_clone</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga29af0ecc2a015ef22289f206bc308d80</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_delete</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga6b3159744a1e5c4ce9d349fd0ebae800</anchor>
      <arglist>(FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_is_equal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga6853bcafe731b1db37105d49f3085349</anchor>
      <arglist>(const FLAC__StreamMetadata *block1, const FLAC__StreamMetadata *block2)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_application_set_data</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga11f340e8877c58d231b09841182d66e5</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *data, uint32_t length, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_resize_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga7352bb944c594f447d3ab316244a9895</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_points)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_seektable_set_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac258246fdda91e14110a186c1d8dcc8c</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num, FLAC__StreamMetadata_SeekPoint point)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_insert_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5ba4c8024988af5985877f9e0b3fef38</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num, FLAC__StreamMetadata_SeekPoint point)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_delete_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa138480c7ea602a31109d3870b41a12f</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t point_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gacd3e1b83fabc1dabccb725b2876c8f53</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_placeholders</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac509d8cb126d06f4bd73505b6c432338</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_point</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0b3aca4fbebc206cd79f13ac36f653f0</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__uint64 sample_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga409f80cb3938814ae307e609faabccc4</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__uint64 sample_numbers[], uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_spaced_points</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab899d58863aa6e974b3ed4ddd2ebf09e</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t num, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_append_spaced_points_by_samples</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab91c8b020a1da37d7524051ae82328cb</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t samples, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_seektable_template_sort</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gafb0449b639ba5c618826d893c2961260</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__bool compact)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_set_vendor_string</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5cf1a57afab200b4b67730a77d3ee162</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_resize_comments</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab44132276cbec9abcadbacafbcd5f92a</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_comments)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_set_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0661d2b99c0e37fd8c5aa673eb302c03</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_insert_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga395fcb4900cd5710e67dc96a9a9cca70</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_append_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga889b8b9c5bbd1070a1214c3da8b72863</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_replace_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga0608308e8c4c09aa610747d8dff90a34</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool all, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_delete_comment</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gac9f51ea4151eb8960e56f31beaa94bd3</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t comment_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab644c34515c04630c62a7645fab2947e</anchor>
      <arglist>(FLAC__StreamMetadata_VorbisComment_Entry *entry, const char *field_name, const char *field_value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_to_name_value_pair</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga29079764fabda53cb3e890e6d05c8345</anchor>
      <arglist>(const FLAC__StreamMetadata_VorbisComment_Entry entry, char **field_name, char **field_value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_vorbiscomment_entry_matches</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaad491f6e73bfb7c5a97b75eda7f4392a</anchor>
      <arglist>(const FLAC__StreamMetadata_VorbisComment_Entry entry, const char *field_name, uint32_t field_name_length)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_find_entry_from</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaeaf925bf881fd4e93bf68ce09b935175</anchor>
      <arglist>(const FLAC__StreamMetadata *object, uint32_t offset, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_remove_entry_matching</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga017d743b3200a27b8567ef33592224b8</anchor>
      <arglist>(FLAC__StreamMetadata *object, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>FLAC__metadata_object_vorbiscomment_remove_entries_matching</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga5a3ff5856098c449622ba850684aec75</anchor>
      <arglist>(FLAC__StreamMetadata *object, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>FLAC__metadata_object_cuesheet_track_new</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gafe2983a9c09685e34626cab39b3fb52c</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata_CueSheet_Track *</type>
      <name>FLAC__metadata_object_cuesheet_track_clone</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga1293d6df6daf2d65143d8bb40eed9261</anchor>
      <arglist>(const FLAC__StreamMetadata_CueSheet_Track *object)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__metadata_object_cuesheet_track_delete</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa533fd7b72fa079e783de4b155b241ce</anchor>
      <arglist>(FLAC__StreamMetadata_CueSheet_Track *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_resize_indices</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga003c90292bc93a877060c34a486fc2b4</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t new_num_indices)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_insert_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga2d66b56b6ebda795ccee86968029e6ad</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num, FLAC__StreamMetadata_CueSheet_Index index)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_insert_blank_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga49ff698f47d914f4e9e45032b3433fba</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_track_delete_index</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gabc751423461062096470b31613468feb</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_resize_tracks</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga9c2edc662e4109c0f8ab5fd72bddaccf</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t new_num_tracks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_set_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gab5f4c6e58c5aa72223e80e7dcdeecfe9</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, FLAC__StreamMetadata_CueSheet_Track *track, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_insert_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaa5e7694a181545251f263fcb672abf3d</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num, FLAC__StreamMetadata_CueSheet_Track *track, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_insert_blank_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga7ccabeffadad2c13522439f1337718ca</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_delete_track</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga241f5d623483b5aebc3a721cce3fa8ec</anchor>
      <arglist>(FLAC__StreamMetadata *object, uint32_t track_num)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_cuesheet_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga1a443d9299ce69694ad59bec4519d7b2</anchor>
      <arglist>(const FLAC__StreamMetadata *object, FLAC__bool check_cd_da_subset, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>FLAC__metadata_object_cuesheet_calculate_cddb_id</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>gaff2f825950b3e4dda4c8ddbf8e2f7ecd</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_mime_type</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga4511ae9ca994c9f4ab035a3c1aa98f45</anchor>
      <arglist>(FLAC__StreamMetadata *object, char *mime_type, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_description</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga293fe7d8b8b9e49d2414db0925b0f442</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *description, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_set_data</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga00c330534ef8336ed92b30f9e676bb5f</anchor>
      <arglist>(FLAC__StreamMetadata *object, FLAC__byte *data, FLAC__uint32 length, FLAC__bool copy)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__metadata_object_picture_is_legal</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga88268a5186e37d4b98b4df7870561128</anchor>
      <arglist>(const FLAC__StreamMetadata *object, const char **violation)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__byte *</type>
      <name>FLAC__metadata_object_get_raw</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga25695e3b6541ed37c94169158cd352f8</anchor>
      <arglist>(const FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamMetadata *</type>
      <name>FLAC__metadata_object_set_raw</name>
      <anchorfile>group__flac__metadata__object.html</anchorfile>
      <anchor>ga2db85cdea68f2fa84dd3f8aa31d9e4eb</anchor>
      <arglist>(FLAC__byte *buffer, FLAC__uint32 length)</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_decoder</name>
    <title>FLAC/ *_decoder.h: decoder interfaces</title>
    <filename>group__flac__decoder.html</filename>
    <subgroup>flac_stream_decoder</subgroup>
  </compound>
  <compound kind="group">
    <name>flac_stream_decoder</name>
    <title>FLAC/stream_decoder.h: stream decoder interface</title>
    <filename>group__flac__stream__decoder.html</filename>
    <class kind="struct">FLAC__StreamDecoder</class>
    <member kind="typedef">
      <type>FLAC__StreamDecoderReadStatus(*</type>
      <name>FLAC__StreamDecoderReadCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga25d4321dc2f122d35ddc9061f44beae7</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderSeekStatus(*</type>
      <name>FLAC__StreamDecoderSeekCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4c18b0216e0f7a83d7e4e7001230545d</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderTellStatus(*</type>
      <name>FLAC__StreamDecoderTellCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gafdf1852486617a40c285c0d76d451a5a</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderLengthStatus(*</type>
      <name>FLAC__StreamDecoderLengthCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga5363f3b46e3f7d6a73385f6560f7e7ef</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__bool(*</type>
      <name>FLAC__StreamDecoderEofCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4eac094fc609363532d90cf8374b4f7e</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamDecoderWriteStatus(*</type>
      <name>FLAC__StreamDecoderWriteCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga61e48dc2c0d2f6c5519290ff046874a4</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamDecoderMetadataCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga6aa87c01744c1c601b7f371f627b6e14</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamDecoderErrorCallback</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac896ee6a12668e9015fab4fbc6aae996</anchor>
      <arglist>)(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)</arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderState</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga3adb6891c5871a87cd5bbae6c770ba2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dacf4455f4f681a6737a553e10f614704a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da4c1853ed1babdcede9a908e12cf7ccf7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daccff915757978117720ba1613d088ddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_FRAME</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da06dc6158a51a8eb9537b65f2fbb6dc49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da28ce845052d9d1a780f4107e97f4c853</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da3bc0343f47153c5779baf7f37f6e95cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daf2c6efcabdfe889081c2260e6681db49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ABORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dadb52ab4785bd2eb84a95e8aa82311cd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da0d08c527252420813e6a6d6d3e19324a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da565eaf4d5e68b440ecec771cb22d3427</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderInitStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaaed54a24ac6310d29c5cafba79759c44</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44ac94c7e9396f30642f34805e5d626e011</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8f2188c616c9bc09638eece3ae55f152</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a798ad4b6c4e556fd4cb1afbc29562eca</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a0110567f0715c6f87357388bc7fa98f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8184c306e0cd2565a8c5adc1381cb469</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a98bc501c9b2fb5d92d8bb0b3321d504f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderReadStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad793ead451206c64a91dc0b851027b93</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a9a5be0fcf0279b98b2fd462bc4871d06</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a0a0687d25dc9f7163e6e5e294672170f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a923123aebb349e35662e35a7621b7535</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderSeekStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac8d269e3c7af1a5889d3bd38409ed67d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67daca58132d896ad7755827d3f2b72488cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da969ce92a42a2a95609452e9cf01fcc09</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da4a01f1e48baf015e78535cc20683ec53</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderTellStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga83708207969383bd7b5c1e9148528845</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845a516a202ebf4bb61d4a1fb5b029a104dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845aceefd3feb853d5e68a149f2bdd1a9db1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845add75538234493c9f7a20a846a223ca91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderLengthStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad5860157c2bb34501b8b9370472d727a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aaef01bfcdc3099686e106d8f88397653d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aab000e31c0c20c0d19df4f2203b01ea23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aae35949f46f887e6d826fe0fe4b2a32c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderWriteStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga73f67eb9e0ab57945afe038751bc62c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8acea48326e0ab8370d2814f4126fcb84e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8a23bd6bfec34af704e0d5ea273f14d95d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamDecoderErrorStatus</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga130e70bd9a73d3c2416247a3e5132ecf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa3ceec2a553dc142ad487ae88eb6f7222</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfae393a9b91a6b2f23398675b5b57e1e86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa208fe77a04e6ff684e50f0eae1214e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa8b6864ad65edd8fea039838b6d3e5575</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa67ee497c6fe564b50d7a7964ef5cd30a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dacf4455f4f681a6737a553e10f614704a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da4c1853ed1babdcede9a908e12cf7ccf7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daccff915757978117720ba1613d088ddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_FRAME</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da06dc6158a51a8eb9537b65f2fbb6dc49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da28ce845052d9d1a780f4107e97f4c853</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da3bc0343f47153c5779baf7f37f6e95cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2daf2c6efcabdfe889081c2260e6681db49</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ABORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2dadb52ab4785bd2eb84a95e8aa82311cd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da0d08c527252420813e6a6d6d3e19324a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga3adb6891c5871a87cd5bbae6c770ba2da565eaf4d5e68b440ecec771cb22d3427</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44ac94c7e9396f30642f34805e5d626e011</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8f2188c616c9bc09638eece3ae55f152</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a798ad4b6c4e556fd4cb1afbc29562eca</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a0110567f0715c6f87357388bc7fa98f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a8184c306e0cd2565a8c5adc1381cb469</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggaaed54a24ac6310d29c5cafba79759c44a98bc501c9b2fb5d92d8bb0b3321d504f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a9a5be0fcf0279b98b2fd462bc4871d06</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a0a0687d25dc9f7163e6e5e294672170f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad793ead451206c64a91dc0b851027b93a923123aebb349e35662e35a7621b7535</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67daca58132d896ad7755827d3f2b72488cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da969ce92a42a2a95609452e9cf01fcc09</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggac8d269e3c7af1a5889d3bd38409ed67da4a01f1e48baf015e78535cc20683ec53</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845a516a202ebf4bb61d4a1fb5b029a104dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845aceefd3feb853d5e68a149f2bdd1a9db1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga83708207969383bd7b5c1e9148528845add75538234493c9f7a20a846a223ca91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_OK</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aaef01bfcdc3099686e106d8f88397653d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aab000e31c0c20c0d19df4f2203b01ea23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ggad5860157c2bb34501b8b9370472d727aae35949f46f887e6d826fe0fe4b2a32c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8acea48326e0ab8370d2814f4126fcb84e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_WRITE_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga73f67eb9e0ab57945afe038751bc62c8a23bd6bfec34af704e0d5ea273f14d95d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa3ceec2a553dc142ad487ae88eb6f7222</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfae393a9b91a6b2f23398675b5b57e1e86</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa208fe77a04e6ff684e50f0eae1214e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa8b6864ad65edd8fea039838b6d3e5575</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gga130e70bd9a73d3c2416247a3e5132ecfa67ee497c6fe564b50d7a7964ef5cd30a</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoder *</type>
      <name>FLAC__stream_decoder_new</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga529c3c1e46417570767fb8e4c76f5477</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_decoder_delete</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad9cf299956da091111d13e83517d8c44</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_ogg_serial_number</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga7fd232e7a2b5070bd26450487edbc2a1</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, long serial_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_md5_checking</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga8f402243eed54f400ddd2f296ff54497</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad4e685f3d055f70fbaed9ffa4f70f74b</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond_application</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaee1196ff5fa97df9810f708dc2bc8326</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_respond_all</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga1ce03d8f305a818ff9a573473af99dc4</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad75f067720da89c4e9d96dedc45f73e6</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore_application</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaab41e8bc505b24df4912de53de06b085</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_set_metadata_ignore_all</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaa1307f07fae5d7a4a0c18beeae7ec5e6</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderState</type>
      <name>FLAC__stream_decoder_get_state</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaf99dac2d9255f7db4df8a6d9974a9a9a</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>FLAC__stream_decoder_get_resolved_state_string</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gad28257412951ca266751a19e2cf54be2</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_get_md5_checking</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gae27a6b30b55beda03559c12a5df21537</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint64</type>
      <name>FLAC__stream_decoder_get_total_samples</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga930d9b591fcfaea74359c722cdfb980c</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_channels</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga802d5f4c48a711b690d6d66d2e3f20a5</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__ChannelAssignment</type>
      <name>FLAC__stream_decoder_get_channel_assignment</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gae62fdf93c1fedd5fea9258ecdc78bb53</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_bits_per_sample</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga689893cde90c171ca343192e92679842</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_sample_rate</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga95f7cdfefba169d964e3c08672a0f0ad</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_decoder_get_blocksize</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gafe07ad9949cc54944fd369fe9335c4bc</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_get_decode_position</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaffd9b0d0832ed01e6d75930b5391def5</anchor>
      <arglist>(const FLAC__StreamDecoder *decoder, FLAC__uint64 *position)</arglist>
    </member>
    <member kind="function">
      <type>const void *</type>
      <name>FLAC__stream_decoder_get_client_data</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gab14d8d4fa1a66a5a603f96090c2deb07</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga150d381abc5249168e439bc076544b29</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderReadCallback read_callback, FLAC__StreamDecoderSeekCallback seek_callback, FLAC__StreamDecoderTellCallback tell_callback, FLAC__StreamDecoderLengthCallback length_callback, FLAC__StreamDecoderEofCallback eof_callback, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga1b043adeb805c779c1e97cb68959d1ab</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderReadCallback read_callback, FLAC__StreamDecoderSeekCallback seek_callback, FLAC__StreamDecoderTellCallback tell_callback, FLAC__StreamDecoderLengthCallback length_callback, FLAC__StreamDecoderEofCallback eof_callback, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga80aa83631460a53263c84e654586dff0</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FILE *file, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_FILE</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4cc7fbaf905c24d6db48b53b7942fe72</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FILE *file, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_file</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga4021ead5cff29fd589c915756f902f1a</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const char *filename, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderInitStatus</type>
      <name>FLAC__stream_decoder_init_ogg_file</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga548f15d7724f3bff7f2608abe8b12f6c</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, const char *filename, FLAC__StreamDecoderWriteCallback write_callback, FLAC__StreamDecoderMetadataCallback metadata_callback, FLAC__StreamDecoderErrorCallback error_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_finish</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga96c47c96920f363cd0972b54067818a9</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_flush</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga95570a455e582b2ab46ab9bb529f26ac</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_reset</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gaa4183c2d925d5a5edddde9d1ca145725</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_single</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga9d6df4a39892c05955122cf7f987f856</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_until_end_of_metadata</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga027ffb5b75dc39b3d26f55c5e6b42682</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_process_until_end_of_stream</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga89a0723812fa6ef7cdb173715f1bc81f</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_skip_single_frame</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga85b666aba976f29e8dd9d7956fce4301</anchor>
      <arglist>(FLAC__StreamDecoder *decoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_decoder_seek_absolute</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga6a2eb6072b9fafefc3f80f1959805ccb</anchor>
      <arglist>(FLAC__StreamDecoder *decoder, FLAC__uint64 sample)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderStateString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac192360ac435614394bf43235cb7981e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderInitStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga0effa1d3031c3206a1719faf984a4f21</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderReadStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gab1ee941839b05045ae1d73ee0fdcb8c9</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderSeekStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac49aff0593584b7ed5fd0b2508f824fc</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderTellStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga3c1b7d5a174d6c2e6bcf1b9a87b5a5cb</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderLengthStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga792933fa9e8b65bfcac62d82e52415f5</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderWriteStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>ga9df7f0fd8cf9888f97a52b5f3f33cdb0</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamDecoderErrorStatusString</name>
      <anchorfile>group__flac__stream__decoder.html</anchorfile>
      <anchor>gac428c69b084529322df05ee793440b88</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flac_encoder</name>
    <title>FLAC/ *_encoder.h: encoder interfaces</title>
    <filename>group__flac__encoder.html</filename>
    <subgroup>flac_stream_encoder</subgroup>
  </compound>
  <compound kind="group">
    <name>flac_stream_encoder</name>
    <title>FLAC/stream_encoder.h: stream encoder interface</title>
    <filename>group__flac__stream__encoder.html</filename>
    <class kind="struct">FLAC__StreamEncoder</class>
    <member kind="typedef">
      <type>FLAC__StreamEncoderReadStatus(*</type>
      <name>FLAC__StreamEncoderReadCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga18b7941b93bae067192732e913536d44</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__byte buffer[], size_t *bytes, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderWriteStatus(*</type>
      <name>FLAC__StreamEncoderWriteCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2998a0af774d793928a7cc3bbc84dcdf</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderSeekStatus(*</type>
      <name>FLAC__StreamEncoderSeekCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga70b85349d5242e4401c4d8ddf6d9bbca</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>FLAC__StreamEncoderTellStatus(*</type>
      <name>FLAC__StreamEncoderTellCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gabefdf2279e1d0347d9f98f46da4e415b</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamEncoderMetadataCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga091fbf3340d85bcbda1090c31bc320cf</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, const FLAC__StreamMetadata *metadata, void *client_data)</arglist>
    </member>
    <member kind="typedef">
      <type>void(*</type>
      <name>FLAC__StreamEncoderProgressCallback</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga42a5fab5f91c1b0c3f7098499285f277</anchor>
      <arglist>)(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, uint32_t frames_written, uint32_t total_frames_estimate, void *client_data)</arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderState</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac5e9db4fc32ca2fa74abd9c8a87c02a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a3a6666ae61a64d955341cec285695bf6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a04912e04a3c57d3c53de34742f96d635</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5abb312cc8318c7a541cadacd23ceb3bbb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_DECODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a4cb80be4f83eb71f04e74968af1d259e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_MISMATCH_IN_AUDIO_DATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a011e3d8b2d02a940bfd0e59c05cf5ae0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_CLIENT_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a8c2b2e9efb43a4f9b25b1d2bd9af5f23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_IO_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5af0e4738522e05a7248435c7148f58f91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_FRAMING_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a2c2937b7f1600a4ac7c84fc70ab34cf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a35db99d9958bd6c2301a04715fbc44fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderInitStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3bb869620af2b188d77982a5c30b047d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da20501dce552da74c5df935eeaa0c9ee3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ENCODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da9c64e5f9020d8799e1cd9d39d50e6955</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da8a822b011de88b67c114505ffef39327</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dac2cf461f02e20513003b8cadeae03f9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_NUMBER_OF_CHANNELS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da0541c4f827f081b9f1c54c9441e4aa65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dad6d2631f464183c0c165155200882e6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_SAMPLE_RATE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da6fdcde9e18c37450c79e8f12b9d9c134</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BLOCK_SIZE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da652c445f1bd8b6cfb963a30bf416c95a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_MAX_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da38a69e94b3333e4ba779d2ff8f43f64e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da5be80403bd7a43450139442e0f34ad7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_BLOCK_SIZE_TOO_SMALL_FOR_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da62a17a3ed3c05ddf8ea7f6fecbd4e4a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa793405c858c7606539082750080a47e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_METADATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa85afdd1849c75a19594416cef63e3e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dab4e7b50d176a127575df90383cb15e1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderReadStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2e81f007fb0a7414c0bbb453f37ea37f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa4bdd691d3666f19ec96ff99402347a2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa562fef84bf86a9a39682e23066d9cfee</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa69b94eeab60e07d5fd33f2b3c8b85759</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa9bb730b8f6354cc1e810017a2f700316</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderWriteStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3737471fd49730bb8cf9b182bdeda05e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea5622e0199f0203c402fcb7b4ca76f808</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea18e7cd6a443fb8bd303c3ba89946bc85</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderSeekStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6d5be3489f45fcf0c252022c65d87aca</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa99853066610d798627888ec2e5afa667</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaabf93227938b4e1bf3656fe4ba4159c60</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa8930179a426134caf30a70147448f037</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>FLAC__StreamEncoderTellStatus</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab628f63181250eb977a28bf12b7dd9ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa48e071d89494ac8f5471e7c0d7a6f43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffaf638882e04d7c58e6c29dcc7f410864b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa9d6bbd317f85fd2d6fc72f64e3cb56e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a3a6666ae61a64d955341cec285695bf6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_UNINITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a04912e04a3c57d3c53de34742f96d635</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_OGG_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5abb312cc8318c7a541cadacd23ceb3bbb</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_DECODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a4cb80be4f83eb71f04e74968af1d259e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_VERIFY_MISMATCH_IN_AUDIO_DATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a011e3d8b2d02a940bfd0e59c05cf5ae0</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_CLIENT_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a8c2b2e9efb43a4f9b25b1d2bd9af5f23</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_IO_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5af0e4738522e05a7248435c7148f58f91</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_FRAMING_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a2c2937b7f1600a4ac7c84fc70ab34cf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_MEMORY_ALLOCATION_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggac5e9db4fc32ca2fa74abd9c8a87c02a5a35db99d9958bd6c2301a04715fbc44fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da20501dce552da74c5df935eeaa0c9ee3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ENCODER_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da9c64e5f9020d8799e1cd9d39d50e6955</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_UNSUPPORTED_CONTAINER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da8a822b011de88b67c114505ffef39327</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_CALLBACKS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dac2cf461f02e20513003b8cadeae03f9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_NUMBER_OF_CHANNELS</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da0541c4f827f081b9f1c54c9441e4aa65</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BITS_PER_SAMPLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dad6d2631f464183c0c165155200882e6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_SAMPLE_RATE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da6fdcde9e18c37450c79e8f12b9d9c134</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BLOCK_SIZE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da652c445f1bd8b6cfb963a30bf416c95a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_MAX_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da38a69e94b3333e4ba779d2ff8f43f64e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_QLP_COEFF_PRECISION</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da5be80403bd7a43450139442e0f34ad7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_BLOCK_SIZE_TOO_SMALL_FOR_LPC_ORDER</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047da62a17a3ed3c05ddf8ea7f6fecbd4e4a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa793405c858c7606539082750080a47e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_METADATA</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047daa85afdd1849c75a19594416cef63e3e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_INIT_STATUS_ALREADY_INITIALIZED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3bb869620af2b188d77982a5c30b047dab4e7b50d176a127575df90383cb15e1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_CONTINUE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa4bdd691d3666f19ec96ff99402347a2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_END_OF_STREAM</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa562fef84bf86a9a39682e23066d9cfee</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_ABORT</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa69b94eeab60e07d5fd33f2b3c8b85759</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_READ_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga2e81f007fb0a7414c0bbb453f37ea37fa9bb730b8f6354cc1e810017a2f700316</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea5622e0199f0203c402fcb7b4ca76f808</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga3737471fd49730bb8cf9b182bdeda05ea18e7cd6a443fb8bd303c3ba89946bc85</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa99853066610d798627888ec2e5afa667</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaabf93227938b4e1bf3656fe4ba4159c60</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gga6d5be3489f45fcf0c252022c65d87acaa8930179a426134caf30a70147448f037</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_OK</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa48e071d89494ac8f5471e7c0d7a6f43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_ERROR</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffaf638882e04d7c58e6c29dcc7f410864b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ggab628f63181250eb977a28bf12b7dd9ffa9d6bbd317f85fd2d6fc72f64e3cb56e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoder *</type>
      <name>FLAC__stream_encoder_new</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab09f7620a0ba9c30020c189ce112a52f</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_encoder_delete</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7212e6846f543618b6289666de216b29</anchor>
      <arglist>(FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_ogg_serial_number</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf4f75f7689b6b3fff16b03028aa38326</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, long serial_number)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_verify</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga795be6527a9eb1219331afef2f182a41</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_streamable_subset</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga35a18815a58141b88db02317892d059b</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_channels</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9ec612a48f81805eafdb059548cdaf92</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_bits_per_sample</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7453fc29d7e86b499f23b1adfba98da1</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_sample_rate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaa6b6537875900a6e0f4418a504f55f25</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_compression_level</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaacc01aab02849119f929b8516420fcd3</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_blocksize</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac35cb1b5614464658262e684c4ac3a2f</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3bff001a1efc2e4eb520c954066330f4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_loose_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7965d51b93f14cbd6ad5bb9d34f10536</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_apodization</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6598f09ac782a1f2a5743ddf247c81c8</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *specification)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_max_lpc_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gad8a0ff058c46f9ce95dc0508f4bdfb0c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_qlp_coeff_precision</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga179751f915a3d6fc2ca4b33a67bb8780</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_qlp_coeff_prec_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga495890067203958e5d67a641f8757b1c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_escape_coding</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaed594c373d829f77808a935c54a25fa4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_do_exhaustive_model_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga054313e7f6eaf5c6122d82c6a8b3b808</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_min_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga8f2ed5a2b35bfea13e6605b0fe55f0fa</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_max_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab9e02bfbbb1d4fcdb666e2e9a678b4f6</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_rice_parameter_search_dist</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga2cc4a05caba8a4058f744d9eb8732caa</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, uint32_t value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_total_samples_estimate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gab943094585d1c0a4bec497e73567cf85</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__uint64 value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_metadata</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga80d57f9069e354cbf1a15a3e3ad9ca78</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamMetadata **metadata, uint32_t num_blocks)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_set_limit_min_bitrate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gac8c5f361b441d528b7a6791b66bb9d40</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__bool value)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderState</type>
      <name>FLAC__stream_encoder_get_state</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0803321b37189dc5eea4fe1cea25c29a</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamDecoderState</type>
      <name>FLAC__stream_encoder_get_verify_decoder_state</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga820704b95a711e77d55363e8753f9f9f</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>FLAC__stream_encoder_get_resolved_state_string</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0916f813358eb6f1e44148353acd4d42</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FLAC__stream_encoder_get_verify_decoder_error_stats</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga28373aaf2c47336828d5672696c36662</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_sample, uint32_t *frame_number, uint32_t *channel, uint32_t *sample, FLAC__int32 *expected, FLAC__int32 *got)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_verify</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9efc4964992e001bcec0a8eaedee8d60</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_streamable_subset</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga201e64032ea4298b2379c93652b28245</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_channels</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga412401503141dd42e37831140f78cfa1</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_bits_per_sample</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga169bbf662b2a2df017b93f663deadd1d</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_sample_rate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gae56f27536528f13375ffdd23fa9045f7</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_blocksize</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf8a9715b2d09a6876b8dc104bfd70cdc</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga32da1f89997ab94ce5d677fcd7e24d56</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_loose_mid_side_stereo</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1455859cf3d233bd4dfff86af010f4fa</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_max_lpc_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga5e1d1c9acd3d5a17106b51f0c0107567</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_qlp_coeff_precision</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga909830fb7f4a0a35710452df39c269a3</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_qlp_coeff_prec_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga65bee5a769d4c5fdc95b81c2fb95061c</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_escape_coding</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0c944049800991422c1bfb3b1c0567a5</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_do_exhaustive_model_search</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7bc8b32f58df5564db4b6114cb11042d</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_min_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga4fa722297092aeaebc9d9e743a327d14</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_max_residual_partition_order</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6f5dfbfb5c6e569c4bae5555c9bf87e6</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>FLAC__stream_encoder_get_rice_parameter_search_dist</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaca0e38f283b2772b92da7cb4495d909a</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint64</type>
      <name>FLAC__stream_encoder_get_total_samples_estimate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaa22d8935bd985b9cccf6592160ffc6f2</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_get_limit_min_bitrate</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga741c26084d203ac24d16c875b5d902ac</anchor>
      <arglist>(const FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_stream</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga7d801879812b48fcbc40f409800c453c</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamEncoderWriteCallback write_callback, FLAC__StreamEncoderSeekCallback seek_callback, FLAC__StreamEncoderTellCallback tell_callback, FLAC__StreamEncoderMetadataCallback metadata_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_stream</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9d1981bcd30b8db4d73b5466be5570f5</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FLAC__StreamEncoderReadCallback read_callback, FLAC__StreamEncoderWriteCallback write_callback, FLAC__StreamEncoderSeekCallback seek_callback, FLAC__StreamEncoderTellCallback tell_callback, FLAC__StreamEncoderMetadataCallback metadata_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_FILE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga12789a1c4a4e31cd2e7187259fe127f8</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FILE *file, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_FILE</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga57fc668f50ffd99a93df326bfab5e2b1</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, FILE *file, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_file</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9d5117c2ac0eeb572784116bf2eb541b</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *filename, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__StreamEncoderInitStatus</type>
      <name>FLAC__stream_encoder_init_ogg_file</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga4891de2f56045941ae222b61b0fd83a4</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const char *filename, FLAC__StreamEncoderProgressCallback progress_callback, void *client_data)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_finish</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga3522f9de5af29807df1b9780a418b7f3</anchor>
      <arglist>(FLAC__StreamEncoder *encoder)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_process</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga87b9c361292da5c5928a8fb5fda7c423</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const FLAC__int32 *const buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="function">
      <type>FLAC__bool</type>
      <name>FLAC__stream_encoder_process_interleaved</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga6e31c221f7e23345267c52f53c046c24</anchor>
      <arglist>(FLAC__StreamEncoder *encoder, const FLAC__int32 buffer[], uint32_t samples)</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderStateString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1410b7a076b0c8401682f9f812b66df5</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderInitStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga0ec1fa7b3f55b4f07a2727846c285776</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderReadStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga1654422c81846b9b399ac5fb98df61dd</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderWriteStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>ga9f64480accd01525cbfa25c11e6bb74e</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderSeekStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gabb137b2d787756bf97398f0b60e54c20</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>const char *const</type>
      <name>FLAC__StreamEncoderTellStatusString</name>
      <anchorfile>group__flac__stream__encoder.html</anchorfile>
      <anchor>gaf8ab921ae968be2be255be1f136e1eec</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flacpp</name>
    <title>FLAC C++ API</title>
    <filename>group__flacpp.html</filename>
    <subgroup>flacpp_decoder</subgroup>
    <subgroup>flacpp_encoder</subgroup>
    <subgroup>flacpp_export</subgroup>
    <subgroup>flacpp_metadata</subgroup>
  </compound>
  <compound kind="group">
    <name>flacpp_decoder</name>
    <title>FLAC++/decoder.h: decoder classes</title>
    <filename>group__flacpp__decoder.html</filename>
    <class kind="class">FLAC::Decoder::Stream</class>
    <class kind="class">FLAC::Decoder::File</class>
  </compound>
  <compound kind="group">
    <name>flacpp_encoder</name>
    <title>FLAC++/encoder.h: encoder classes</title>
    <filename>group__flacpp__encoder.html</filename>
    <class kind="class">FLAC::Encoder::Stream</class>
    <class kind="class">FLAC::Encoder::File</class>
  </compound>
  <compound kind="group">
    <name>flacpp_export</name>
    <title>FLAC++/export.h: export symbols</title>
    <filename>group__flacpp__export.html</filename>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gaec3a801bf18630403eda6dc2f8c4927a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_CURRENT</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gafc3064beba20c1795d8aaa801b79d3b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_REVISION</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>gaebce36e5325dbdcdc1a9e61a44606efe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLACPP_API_VERSION_AGE</name>
      <anchorfile>group__flacpp__export.html</anchorfile>
      <anchor>ga17d0e89a961696b32c2b11e08663543f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flacpp_metadata</name>
    <title>FLAC++/metadata.h: metadata interfaces</title>
    <filename>group__flacpp__metadata.html</filename>
    <subgroup>flacpp_metadata_object</subgroup>
  </compound>
  <compound kind="group">
    <name>flacpp_metadata_object</name>
    <title>FLAC++/metadata.h: metadata object classes</title>
    <filename>group__flacpp__metadata__object.html</filename>
    <subgroup>flacpp_metadata_level0</subgroup>
    <class kind="class">FLAC::Metadata::Prototype</class>
    <class kind="class">FLAC::Metadata::StreamInfo</class>
    <class kind="class">FLAC::Metadata::Padding</class>
    <class kind="class">FLAC::Metadata::Application</class>
    <class kind="class">FLAC::Metadata::SeekTable</class>
    <class kind="class">FLAC::Metadata::VorbisComment</class>
    <class kind="class">FLAC::Metadata::CueSheet</class>
    <class kind="class">FLAC::Metadata::Picture</class>
    <class kind="class">FLAC::Metadata::Unknown</class>
    <member kind="function">
      <type>Prototype *</type>
      <name>clone</name>
      <anchorfile>group__flacpp__metadata__object.html</anchorfile>
      <anchor>gae18d91726a320349b2c3fb45e79d21fc</anchor>
      <arglist>(const Prototype *)</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flacpp_metadata_level0</name>
    <title>FLAC++/metadata.h: metadata level 0 interface</title>
    <filename>group__flacpp__metadata__level0.html</filename>
    <subgroup>flacpp_metadata_level1</subgroup>
    <member kind="function">
      <type>bool</type>
      <name>get_streaminfo</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga8fa8da652f33edeb4dabb4ce39fda04b</anchor>
      <arglist>(const char *filename, StreamInfo &amp;streaminfo)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_tags</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga533a71ba745ca03068523a4a45fb0329</anchor>
      <arglist>(const char *filename, VorbisComment *&amp;tags)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_tags</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga85166e6206f3d5635684de4257f2b00e</anchor>
      <arglist>(const char *filename, VorbisComment &amp;tags)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_cuesheet</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>ga4fad03d91f22d78acf35dd2f35df9ac7</anchor>
      <arglist>(const char *filename, CueSheet *&amp;cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_cuesheet</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaea8f05f89e36af143d73b4280f05cc0e</anchor>
      <arglist>(const char *filename, CueSheet &amp;cuesheet)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_picture</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaa44df95da4d3abc459fdc526a0d54a55</anchor>
      <arglist>(const char *filename, Picture *&amp;picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_picture</name>
      <anchorfile>group__flacpp__metadata__level0.html</anchorfile>
      <anchor>gaa6aea22f1ebeb671db19b73277babdea</anchor>
      <arglist>(const char *filename, Picture &amp;picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>flacpp_metadata_level1</name>
    <title>FLAC++/metadata.h: metadata level 1 interface</title>
    <filename>group__flacpp__metadata__level1.html</filename>
    <subgroup>flacpp_metadata_level2</subgroup>
    <class kind="class">FLAC::Metadata::SimpleIterator</class>
  </compound>
  <compound kind="group">
    <name>flacpp_metadata_level2</name>
    <title>FLAC++/metadata.h: metadata level 2 interface</title>
    <filename>group__flacpp__metadata__level2.html</filename>
    <class kind="class">FLAC::Metadata::VorbisComment::Entry</class>
    <class kind="class">FLAC::Metadata::CueSheet::Track</class>
    <class kind="class">FLAC::Metadata::SimpleIterator::Status</class>
    <class kind="class">FLAC::Metadata::Chain</class>
    <class kind="class">FLAC::Metadata::Chain::Status</class>
    <class kind="class">FLAC::Metadata::Iterator</class>
    <member kind="function" protection="protected">
      <type></type>
      <name>Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gae49fa399a6273ccad7cb0e6f787a3f5c</anchor>
      <arglist>(const Prototype &amp;)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga23ec8d118119578adb95de42fcbbaca2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaea76819568855c4f49f2a23d42a642f2</anchor>
      <arglist>(const Prototype &amp;)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>Prototype &amp;</type>
      <name>assign_object</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacc8ddaac1f1afe9d4fd9de33354847bd</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>clear</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa54338931745f7f1b1d8240441efedb8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~Prototype</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga698fa1529af534ab5d1d98d0979844f6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5f1ce22db46834e315363e730f24ffaf</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab8e067674ea0181dc0756bbb5b242c6e</anchor>
      <arglist>(const Prototype &amp;) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0466615f2d7e725d1fc33bd1ae72ea5b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad88ba607c1bb6b3729b4a729be181db8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga524f81715c9aae70ba8b1b7ee4565171</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d95592dea00bcf47dcdbc0b7224cf9e</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf40c7c078e408f7d6d0b5f521a013315</anchor>
      <arglist>(bool)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator const ::FLAC__StreamMetadata *</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga72cc341e319780e2dca66d7c28bd0200</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>construct_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8df28d7c46448436905e52a01824dbec</anchor>
      <arglist>(::FLAC__StreamMetadata *object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>StreamInfo</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab86611073f13dd3e7aea386bb6f1a7a4</anchor>
      <arglist>(const StreamInfo &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>StreamInfo</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaaf4d96124e2b323398f7edf1aaf28003</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>StreamInfo &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga353a63aa812f125fedec844142946142</anchor>
      <arglist>(const StreamInfo &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>StreamInfo &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad1193a408a5735845dea17a131b7282c</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga4010b479ff46aad5ddd363bf456fbfa1</anchor>
      <arglist>(const StreamInfo &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf0f86d918ae7416e4de77215df6e861b</anchor>
      <arglist>(const StreamInfo &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_min_blocksize</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa53c8d9f0c5c396a51bbf543093121cc</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3a5665a824530dec2906d76e665573ee</anchor>
      <arglist>(const Padding &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga358085e3cec897ed0b0c88c8ac04618d</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga86b26d7f7df2a1b3ee0215f2b9352274</anchor>
      <arglist>(uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>Padding &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaece6ab03932bea3f0c32ff3cd88f2617</anchor>
      <arglist>(const Padding &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Padding &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3b7508e56df71854ff1f5ad9570b5684</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1c400bb08e873eae7a1a8640a97d4cde</anchor>
      <arglist>(const Padding &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga12654720889aec7a4694c97f2b1f75b7</anchor>
      <arglist>(const Padding &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga07dae9d71b724f27f4bfbea26d7ab8fc</anchor>
      <arglist>(uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Application</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac852c4aa3be004f1ffa4895ca54354a0</anchor>
      <arglist>(const Application &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Application</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga354471e537af33ba0c86de4db988efd1</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Application &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3ca9dd06666b1dc7d4bdb6aef8e14d04</anchor>
      <arglist>(const Application &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Application &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga47f68d7001ef094a916d3b13fe589fc2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga89c2e1e78226550b47fceb2ab7fe1fa8</anchor>
      <arglist>(const Application &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadf4f2c38053d0d39e735c5f30b9934cf</anchor>
      <arglist>(const Application &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga95eaa06ca65af25385cf05f4942100b8</anchor>
      <arglist>(const FLAC__byte *data, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SeekTable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7f93d054937829a85108cd423a56299f</anchor>
      <arglist>(const SeekTable &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SeekTable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaccd82ef77dcc489280c0f46e443b16c7</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>SeekTable &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac1094c0536952a569e41ba619f9b4ff5</anchor>
      <arglist>(const SeekTable &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>SeekTable &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad9d0036938d6ad1c81180cf1e156b844</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga15966f2e33461ce14c3d98a41d47f94d</anchor>
      <arglist>(const SeekTable &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9b25b057f2fdbdc88e2db66d94ad0de4</anchor>
      <arglist>(const SeekTable &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga09783f913385728901ff93686456d647</anchor>
      <arglist>(uint32_t new_num_points)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad01b009dc3aecd5e881b7b425439643f</anchor>
      <arglist>(uint32_t index, const ::FLAC__StreamMetadata_SeekPoint &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gabc1476cf5960660fa5c5d4a65db1441f</anchor>
      <arglist>(uint32_t index, const ::FLAC__StreamMetadata_SeekPoint &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0d8260db8b7534cc66fe2b80380c91bd</anchor>
      <arglist>(uint32_t index)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8a47e1f8b8331024c2ae977d8bd104d6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_placeholders</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gae8e334f73f3d8870df2e948aa5de1234</anchor>
      <arglist>(uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_point</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9c05d6c010988cf2f336ab1c02c3c618</anchor>
      <arglist>(FLAC__uint64 sample_number)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad3c644c5e7de6b944feee725d396b27e</anchor>
      <arglist>(FLAC__uint64 sample_numbers[], uint32_t num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_spaced_points</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga6ecfcb2478134b483790276b22a4f8b2</anchor>
      <arglist>(uint32_t num, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_append_spaced_points_by_samples</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga64bc1300d59e79f6c99356bf4a256383</anchor>
      <arglist>(uint32_t samples, FLAC__uint64 total_samples)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>template_sort</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga09cc5c101fc9c26655de9ec91dcb502f</anchor>
      <arglist>(bool compact)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga75772bb6b5bf90da459e7fb247239b27</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>VorbisComment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga436a5c6a42a83a88206376805743fe3b</anchor>
      <arglist>(const VorbisComment &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>VorbisComment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga65a73f4665db16ac7aec76e9f5e699f2</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>VorbisComment &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga135650367ce6c2c5ce12b534307f1cca</anchor>
      <arglist>(const VorbisComment &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>VorbisComment &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9db2171c398cd62a5907e625c3a6228d</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga40e48312009df9d321a46df47fceb63b</anchor>
      <arglist>(const VorbisComment &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac882ee4619675b1231d38a58af5fc8a8</anchor>
      <arglist>(const VorbisComment &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_vendor_string</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad8cffdb4c43ba01eaa9a3f7be0d5926a</anchor>
      <arglist>(const FLAC__byte *string)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_comments</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad924744735bfd0dad8a30aabe2865cbb</anchor>
      <arglist>(uint32_t new_num_comments)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad179979211b6f4ed4ca0e8df0760b343</anchor>
      <arglist>(uint32_t index, const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab1de71f1c0acdc93c1ed39b6b5e09956</anchor>
      <arglist>(uint32_t index, const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>append_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1126c7a0f25a2cf78efc8317d3a861f2</anchor>
      <arglist>(const Entry &amp;entry)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>replace_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga240eb83264d05d953395e75e18e15ee2</anchor>
      <arglist>(const Entry &amp;entry, bool all)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_comment</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf79834672ef87d30faa4574755f05ef8</anchor>
      <arglist>(uint32_t index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_entry_from</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1a8d3eec60ce932566ce847fb7fbb97d</anchor>
      <arglist>(uint32_t offset, const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>remove_entry_matching</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf0770518f35fe18fb9a0cc5c0542c4b7</anchor>
      <arglist>(const char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>remove_entries_matching</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadde2dc584e31f29d67fcc6d15d2d1034</anchor>
      <arglist>(const char *field_name)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac0fb597614c2327157e765ea278b014f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>CueSheet</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaff87fa8ab761fc12c0f37b6ff033f74e</anchor>
      <arglist>(const CueSheet &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>CueSheet</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gadd934e1916c2427197f8a5654f7ffae9</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>CueSheet &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad24bf2e19de81159d5e205ae5ef63843</anchor>
      <arglist>(const CueSheet &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>CueSheet &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac83a472ca9852f3e2e800ae57d3e1305</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad101b9f069c4af9053718b408a9737f5</anchor>
      <arglist>(const CueSheet &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad02b4b1f541c8607a233a248ec295db9</anchor>
      <arglist>(const CueSheet &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_indices</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7dd7822a201fa2310410029a36f4f1ac</anchor>
      <arglist>(uint32_t track_num, uint32_t new_num_indices)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gacebb3ac32324137091b965a9e9ba2edf</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num, const ::FLAC__StreamMetadata_CueSheet_Index &amp;index)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_blank_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga294125ebcaf6c1576759b74f4ba96aa6</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_index</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga01c9f6ec36ba9b538ac3c9de993551f8</anchor>
      <arglist>(uint32_t track_num, uint32_t index_num)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>resize_tracks</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8d574ef586ab17413dbf1cb45b630a69</anchor>
      <arglist>(uint32_t new_num_tracks)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5854e1797bf5161d1dc7e9cca5201bc9</anchor>
      <arglist>(uint32_t i, const Track &amp;track)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaeef4dc2ff2f9cc102855aec900860ce6</anchor>
      <arglist>(uint32_t i, const Track &amp;track)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_blank_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gabe22447cc77d2f12092b68493ad2fca5</anchor>
      <arglist>(uint32_t i)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_track</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga742ea19be39cd5ad23aeac04671c44ae</anchor>
      <arglist>(uint32_t i)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga920da7efb6143683543440c2409b3d26</anchor>
      <arglist>(bool check_cd_da_subset=false, const char **violation=0) const</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>calculate_cddb_id</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7f03abfc2473e54a766c888c8cd431b6</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Picture</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga368985afb060fe1024129ed808392183</anchor>
      <arglist>(const Picture &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Picture</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga703d5d8a88e9764714ee2dd25806e381</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Picture &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2ab3ef473f6c70aafe5bd3229f397a93</anchor>
      <arglist>(const Picture &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Picture &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa3d7384cb724a842c3471a9ab19f81ed</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1cc03a87e1ada7b81af2dfe487d86fa7</anchor>
      <arglist>(const Picture &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3b0c4fa11c7c54427e7aa690c8998692</anchor>
      <arglist>(const Picture &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>FLAC__uint32</type>
      <name>get_colors</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab44cabf75add1973ebde9f5f7ed6b780</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_mime_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gafb4e53cb8ae62ea0d9ebd1afdca40c3f</anchor>
      <arglist>(const char *string)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_description</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1bbcd96802a16fc36ac1b6610cd7d4a3</anchor>
      <arglist>(const FLAC__byte *string)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_colors</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga8e7dc667ccc55e60abe2b8a751656097</anchor>
      <arglist>(FLAC__uint32 value) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga301630d1c8f7647d0f192e6a2a03e6ba</anchor>
      <arglist>(const FLAC__byte *data, FLAC__uint32 data_length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_legal</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaf11147e2041b46d679b077e6ac26bea0</anchor>
      <arglist>(const char **violation)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Unknown</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga686a799c353cf7a3dc95bb8899318a6b</anchor>
      <arglist>(const Unknown &amp;object)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Unknown</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2fb76f94e891c3eea7209a461cab4279</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>Unknown &amp;</type>
      <name>operator=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga295f824df8ed10c3386df72272fdca47</anchor>
      <arglist>(const Unknown &amp;object)</arglist>
    </member>
    <member kind="function">
      <type>Unknown &amp;</type>
      <name>assign</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga4dc5e794c8d529245888414b2bf7d404</anchor>
      <arglist>(::FLAC__StreamMetadata *object, bool copy)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3a94274ea08f3ff252216b82c07b73e1</anchor>
      <arglist>(const Unknown &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa700239bfb0acd74e7e8ca0b1cdfcdb5</anchor>
      <arglist>(const Unknown &amp;object) const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_data</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad9618a004195b86f5989f5f0d396d028</anchor>
      <arglist>(const FLAC__byte *data, uint32_t length)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga5d528419f9c71d92b71d1d79cff52207</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>init</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67dc75f18d282f41696467f1fbf5c3e8</anchor>
      <arglist>(const char *filename, bool read_only, bool preserve_file_stats)</arglist>
    </member>
    <member kind="function">
      <type>Status</type>
      <name>status</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga9e681b6ad35b10633002ecea5cab37c3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_writable</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga70d7bb568dc6190f9cc5be089eaed03b</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>next</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab399f6b8c5e35a1d18588279613ea63c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>prev</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga75a859af156322f451045418876eb6a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_last</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gac83c8401b2e58a3e4ce03a9996523c44</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>off_t</type>
      <name>get_block_offset</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gad3779538af5b3fe7cdd2188c79bc80b0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_block_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga30dff6debdbc72aceac7a69b9c3bea75</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>get_block_length</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga7e53cef599f3ff984a847a4a251afea5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>get_application_id</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga426d06a9d079f74e82eaa217f14997a5</anchor>
      <arglist>(FLAC__byte *id)</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>get_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab206e5d7145d3726335d336cbc452598</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0ebd4df55346cbcec9ace04f7d7b484d</anchor>
      <arglist>(Prototype *block, bool use_padding=true)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_after</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d0e512147967b7e12ac22914fbe3818</anchor>
      <arglist>(Prototype *block, bool use_padding=true)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67824deff81e2f49c2f51db6b71565e8</anchor>
      <arglist>(bool use_padding=true)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga62ff055714c8ce75d907ae58738113a4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Status</type>
      <name>status</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga02d7a4adc89e37b28eaccbccfe5da5b0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>read</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga509bf6a75a12df65bc77947a4765d9c1</anchor>
      <arglist>(const char *filename, bool is_ogg=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>read</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga030c805328fc8b2da947830959dafb5b</anchor>
      <arglist>(FLAC__IOHandle handle, FLAC__IOCallbacks callbacks, bool is_ogg=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>check_if_tempfile_needed</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d54ed419365faf5429caa84b35265c3</anchor>
      <arglist>(bool use_padding)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga2341690885e2312013afc561e6fafd81</anchor>
      <arglist>(bool use_padding=true, bool preserve_file_stats=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga0ef47e1634bca2d269ac49fc164306b5</anchor>
      <arglist>(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>write</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga37b863c4d490fea96f67294f03fbe975</anchor>
      <arglist>(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks, ::FLAC__IOHandle temp_handle, ::FLAC__IOCallbacks temp_callbacks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>merge_padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaef51a0414284f468a2d73c07b540641d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sort_padding</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga779eaac12da7e7edac67089053e5907f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>is_valid</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga42057c663e277d83cc91763730d38b0f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gab5713af7318f10a46bd8b26ce586947c</anchor>
      <arglist>(Chain &amp;chain)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>next</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga1d2871fc1fdcc5dffee1eafd7019f4a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>prev</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gade6ee6b67b22115959e2adfc65d5d3b4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>::FLAC__MetadataType</type>
      <name>get_block_type</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>gaa25cb3c27e4d6250f98605f89b0fa904</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Prototype *</type>
      <name>get_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3693233f592b9cb333c437413c6be2a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>set_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga3123daf89fca2a8981c9f361f466a418</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>delete_block</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga67adaa4ae39cf405ee0f4674ca8836dd</anchor>
      <arglist>(bool replace_with_padding)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_before</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga86de6d0b21ac08b74a2ea8c1a9adce36</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>insert_block_after</name>
      <anchorfile>group__flacpp__metadata__level2.html</anchorfile>
      <anchor>ga73e7a3f7192f369cb3a19d078da504ab</anchor>
      <arglist>(Prototype *block)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title></title>
    <filename>index.html</filename>
    <docanchor file="index.html" title="Introduction">intro</docanchor>
    <docanchor file="index.html" title="FLAC C API">c_api</docanchor>
    <docanchor file="index.html" title="FLAC C++ API">cpp_api</docanchor>
    <docanchor file="index.html" title="Getting Started">getting_started</docanchor>
    <docanchor file="index.html" title="Porting Guide">porting_guide</docanchor>
    <docanchor file="index.html" title="Embedded Developers">embedded_developers</docanchor>
  </compound>
</tagfile>
