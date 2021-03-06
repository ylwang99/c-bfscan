#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "include/constants.h"
#include "include/data.c"
#include "include/heap.c"

extern void init_tf(char *data_path);
int num_topics;
int num_docs;
int total_terms;

int main(int argc, const char* argv[]) {
  init_tf(argv[1]);

  int n, base, i, j, t;

  clock_t begin, end;
  double time_spent;
  begin = clock();

  for (n = 0; n < num_topics; n ++) {
    heap h;
    heap_create(&h, 0, NULL);
    float* min_key;
    int* min_val;

    float score;

    base = 0;
    for (i = 0; i < num_docs; i ++) {
      if (tweetids[i] > topics_time[n]) {
        base += doclengths_ordered[i];
        continue;
      }

      score = 0;
      // for (j = 0; j < doclengths_ordered[i]; j ++) {
      //   for (t = 2; t < 2 + topics[n][1]; t ++) {
      //     if (collection_tf[base + j] == topics[n][t]) {
      //       score += log(1 + tf[base + j] / (MU * (cf[topics[n][t]] + 1) / (total_terms + 1))) + log(MU / (doclengths[i] + MU));
      //     }
      //   }
      // }

      j = 0;
      t = 2;
      while (j < doclengths_ordered[i] && t < 2 + topics[n][1]) {
        if (collection_tf[base + j] < topics[n][t]) {
          j ++;
        } else if (collection_tf[base + j] > topics[n][t]) {
          t ++;
        } else {
          score += log(1 + tf[base + j] / (MU * (cf[topics[n][t]] + 1) / (total_terms + 1))) + log(MU / (doclengths[i] + MU));
          j ++;
          t ++;
        }
      }

      if (score > 0) {
        int size = heap_size(&h);
        if (size < TOP_K) {
          int *docid = malloc(sizeof(int)); *docid = i;
          float *scorez = malloc(sizeof(float)); *scorez = score;
          heap_insert(&h, scorez, docid);
        } else {
          heap_min(&h, (void**)&min_key, (void**)&min_val);
          if (score > *min_key) {
            heap_delmin(&h, (void**)&min_key, (void**)&min_val);
            int *docid = malloc(sizeof(int)); *docid = i;
            float *scorez = malloc(sizeof(float)); *scorez = score;
            heap_insert(&h, scorez, docid);
          }
        }
      }

      base += doclengths_ordered[i];
    }

    int rank = TOP_K;
    while (heap_delmin(&h, (void**)&min_key, (void**)&min_val)) {
      // printf("MB%02d Q0 %ld %d %f Scan1_TermSorted\n", (n + 1), tweetids[*min_val], rank, *min_key);
      rank --;
    }

    heap_destroy(&h);
  }

  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  // printf("Total time = %f ms\n", time_spent * 1000);
  printf("Time per query = %f ms\n", (time_spent * 1000) / num_topics);
  // printf("Throughput: %f qps\n", num_topics / time_spent);
}