#ifndef DELTA_H_
#define DELTA_H_

#include "delta-file.h"
#include "strbuf.h"
#include "file.h"


/**
 * `delta_options` :
 *    options passed to delta
 */
struct delta_options;

/**
 * `delta_result` :
 *    result container of the delta
 */
struct delta_result;

/**
 * `result_overview` :
 *     overview of the delta's result
 */
struct result_overview;

/**
 * `delta` :
 *     main delta structure
 */
struct delta_input;


#endif