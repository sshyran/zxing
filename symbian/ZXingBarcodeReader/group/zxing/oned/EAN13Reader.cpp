/*
 *  EAN13Reader.cpp
 *  ZXing
 *
 *  Created by Lukasz Warchol on 10-01-22.
 *  Copyright 2010 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "EAN13Reader.h"
#include <zxing/ReaderException.h>

namespace zxing {
	namespace oned {
		
		static const int FIRST_DIGIT_ENCODINGS[10] = {0x00, 0x0B, 0x0D, 0xE, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A};
				
		
		EAN13Reader::EAN13Reader(){
			decodeMiddleCounters = new int[4];
			for (int i=0; i<4; i++) {
				decodeMiddleCounters[i] = 0;
			}
		}
		
		int EAN13Reader::decodeMiddle(Ref<BitArray> row, int startRange[], int startRangeLen, std::string& resultString){
			int countersLen = 4;
			int* counters = decodeMiddleCounters;
			counters[0] = 0;
			counters[1] = 0;
			counters[2] = 0;
			counters[3] = 0;
			
			
			int end = row->getSize();
			int rowOffset = startRange[1];
			
			int lgPatternFound = 0;
			
			for (int x = 0; x < 6 && rowOffset < end; x++) {
				int bestMatch = decodeDigit(row, counters, countersLen, rowOffset, UPC_EAN_PATTERNS_L_AND_G_PATTERNS);
				resultString.append(1, (char) ('0' + bestMatch % 10));
				for (int i = 0; i < countersLen; i++) {
					rowOffset += counters[i];
				}
				if (bestMatch >= 10) {
					lgPatternFound |= 1 << (5 - x);
				}
			}
			
			determineFirstDigit(resultString, lgPatternFound);
			
			int* middleRange = findGuardPattern(row, rowOffset, true, (int*)getMIDDLE_PATTERN(), getMIDDLE_PATTERN_LEN());
			rowOffset = middleRange[1];
			
			for (int x = 0; x < 6 && rowOffset < end; x++) {
				int bestMatch = decodeDigit(row, counters, countersLen, rowOffset, UPC_EAN_PATTERNS_L_PATTERNS);
				resultString.append(1, (char) ('0' + bestMatch));
				for (int i = 0; i < countersLen; i++) {
					rowOffset += counters[i];
				}
			}
			
			return rowOffset;
		}
		
		void EAN13Reader::determineFirstDigit(std::string& resultString, int lgPatternFound){
			for (int d = 0; d < 10; d++) {
				if (lgPatternFound == FIRST_DIGIT_ENCODINGS[d]) {
					resultString.insert((size_t)0, (size_t)1, (char) ('0' + d));
					return;
				}
			}
			throw ReaderException("determineFirstDigit");
		}
		
		BarcodeFormat EAN13Reader::getBarcodeFormat(){
			return BarcodeFormat_EAN_13;
		}
		EAN13Reader::~EAN13Reader(){
			delete [] decodeMiddleCounters;
		}
	}
}