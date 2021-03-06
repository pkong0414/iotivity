/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
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
 *
 ******************************************************************/

#ifndef QUERY_PARAM_GENERATOR_H_
#define QUERY_PARAM_GENERATOR_H_

#include <string>
#include <map>
#include <vector>

/**
 * @class QPGenerator
 */
class QPGenerator
{
    public:
        /** query parameter detail structure */
        typedef struct
        {
            std::string key;                 /**< identity of detail */
            std::vector<std::string> values; /**< parameter value */
            std::size_t index;               /**< index of detail */
        } QPDetail;

        QPGenerator() = default;
        /**
         * This method is for query parameter generator
         * @param[in] queryParams    query parameter
         */
        QPGenerator(const std::map<std::string, std::vector<std::string>> &queryParams);
        /** check the next parameter from list */
        bool hasNext();
        std::map<std::string, std::string> next();

    private:
        std::vector<QPDetail> m_qpDetails;
};

#endif
