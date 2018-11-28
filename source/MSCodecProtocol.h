//
//  MSCodecProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecProtocol_h
#define MSCodecProtocol_h

namespace MS {
    
    class MSDecoderProtocol {
    public:
        virtual ~MSDecoderProtocol() {};
        virtual MSData * const decodeVideo(const MSData &videoData) = 0;
        virtual MSData * const decodeAudio(const MSData &audioData) = 0;
    };
    
    class MSEncoderProtocol {
    public:
        virtual ~MSEncoderProtocol() {};
        virtual MSData * const encodeVideo(const MSData &pixelData) = 0;
        virtual MSData * const encodeAudio(const MSData &sampleData) = 0;
    };
    
}


#endif /* MSCodecProtocol_h */
