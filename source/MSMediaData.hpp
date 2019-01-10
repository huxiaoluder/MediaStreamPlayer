//
//  MSMediaData
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/30.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSMediaData_hpp
#define MSMediaData_hpp

#include <chrono>
#include <cstring>
#include <functional>
#include <type_traits>

#define intervale(rate) microseconds(1000000LL/rate)

#define ErrorLocationLog \
        printf( "-----------------------ERROR----------------------\n"\
        "| filename:    %-s\n"\
        "| linenumber:  %d,\n"\
        "| funcname:    %s\n"\
        "--------------------------------------------------\n",__FILE__,__LINE__,__func__)

namespace MS {
    
    using namespace std;
    
    using namespace chrono;
    
    enum MSCodecID {
        /*---video---*/
        MSCodecID_H264,
        MSCodecID_H265,
        MSCodecID_HEVC = MSCodecID_H265,
        /*---audio---*/
        MSCodecID_AAC,
        MSCodecID_OPUS,
        MSCodecID_ALAW,//G711A
    };
    
    enum MSContentType {
        isDecode,
        isEncode,
    };
    
#pragma mark - MSContent<isEncode, CT>(declaration)
    /*-------------------MSContent<isEncode, CT>(declaration)-------------------*/
    template <MSContentType DT, typename CT = uint8_t>
    struct MSContent {
        // packt data by encoder or from network source, free by player
        CT * const packt;
        
        // packt data size
        const size_t size;
        
        // packt data's encode ID
        const MSCodecID codecID;
        
        MSContent(typename enable_if<is_same<CT, uint8_t>::value,CT>::type * const packt,
                  const size_t size, const MSCodecID codecID);
        
        MSContent(const MSContent &content);
        
        ~MSContent();
    };
    
#pragma mark -- MSContent<isEncode, CT>(implementation) --
    /*-------------------MSContent<isEncode, CT>(implementation)-------------------*/
    template <MSContentType DT, typename CT>
    MSContent<DT,CT>::MSContent(typename enable_if<is_same<CT, uint8_t>::value,CT>::type * const packt,
                                const size_t size, const MSCodecID codecID)
    :packt(new CT[size]), size(size), codecID(codecID) {
        memcpy(this->packt, packt, size);
    }
    
    template <MSContentType DT, typename CT>
    MSContent<DT,CT>::MSContent(const MSContent &content)
    :packt(new CT[content.size]), size(content.size), codecID(content.codecID) {
        memcpy(packt, content.packt, size);
    }
    
    template <MSContentType DT, typename CT>
    MSContent<DT,CT>::~MSContent() {
        delete[] packt;
    }
    
#pragma mark - MSContent<isDecode, CT>(declaration)
    /*-------------------MSContent<isDecode, CT>(declaration)-------------------*/
    template <typename CT>
    struct MSContent<isDecode, CT> {
        // frame data by user's decoder decoded, free by player
        CT * const frame;
        
        // associated with frame rate, to refresh palyer timer's interval dynamically
        const microseconds timeInterval;
        
        // free function, T frame set by user's custom decoder, must provide the decoder's free method
        const function<void(CT * const)> free;
        
        // copy function, T frame set by user's custom decoder, must provide the decoder's copy method
        const function<CT *(CT * const)> copy;
        
        MSContent(typename enable_if<!is_pointer<CT>::value,CT>::type * const frame,
                  const microseconds timeInterval,
                  const function<void(CT * const)> free,
                  const function<CT *(CT * const)> copy);
        
        MSContent(const MSContent &content);
        
        ~MSContent();
    };
    
#pragma mark -- MSContent<isDecode, CT>(implementation) --
    /*-------------------MSContent<isDecode, CT>(implementation)-------------------*/
    template <typename CT>
    MSContent<isDecode,CT>::MSContent(typename enable_if<!is_pointer<CT>::value,CT>::type * const frame,
                                      const microseconds timeInterval,
                                      const function<void(CT * const)> free,
                                      const function<CT *(CT * const)> copy)
    :frame(frame), timeInterval(timeInterval),
    free(free), copy(copy) {
        if (frame) {
            assert(free != nullptr && copy != nullptr);
        }
    }
    
    template <typename CT>
    MSContent<isDecode,CT>::MSContent(const MSContent &content)
    :frame(content.copy(content.frame)), timeInterval(content.timeInterval),
    free(content.free), copy(content.copy) {
        
    }
    
    template <typename CT>
    MSContent<isDecode,CT>::~MSContent() {
        free(frame);
    }
    
#pragma mark - MSMediaData<DT, CT>(declaration)
    /*---------------------MSMediaData<DT, CT>(declaration)---------------------*/
    template <MSContentType DT, typename CT = uint8_t>
    struct MSMediaData {
        
        typedef MSMediaData <isDecode,  CT> MSDecoderOutputData;
        typedef MSContent   <isDecode,  CT> MSDecoderOutputContent;
        
        typedef MSMediaData <isDecode,  CT> MSEncoderInputData;
        typedef MSContent   <isDecode,  CT> MSEncoderInputContent;
        
        typedef MSMediaData<DT, CT> DefaultNullData;
        
        // default null data, for decode data
        static const DefaultNullData defaultNullData;
        
        MSContent<DT, CT> * const content;
        
        MSMediaData(MSContent<DT, CT> * const content);
        
        MSMediaData(const MSMediaData &data);
        
        ~MSMediaData();
        
        MSMediaData * copy();
    };
    
#pragma mark -- MSMediaData<DT, CT>(implementation) --
    /*---------------------MSMediaData<DT, CT>(implementation)---------------------*/
    template <MSContentType DT, typename CT>
    const typename MSMediaData<DT,CT>::DefaultNullData
    MSMediaData<DT,CT>::defaultNullData(nullptr);
    
    template <MSContentType DT, typename CT>
    MSMediaData<DT,CT>::MSMediaData(MSContent<DT, CT> * const content)
    :content(content) {
        
    }
    
    template <MSContentType DT, typename CT>
    MSMediaData<DT,CT>::MSMediaData(const MSMediaData &mediaData)
    :content(mediaData.content) {
        
    }
    
    template <MSContentType DT, typename CT>
    MSMediaData<DT,CT>::~MSMediaData() {
        delete content;
    }
    
    template <MSContentType DT, typename CT>
    MSMediaData<DT,CT> *
    MSMediaData<DT,CT>::copy() {
        return new MSMediaData(*this);
    }
    
}

#endif /* MSMediaData_hpp */


