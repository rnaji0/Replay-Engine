# Test Notes

## Definitions
A pass rate is defined as the number of test which passed divided by the total number of tests that occured. We determine whether a test passes or fails by comparing a test's observed output with the output of a calculated determinstic operation.

It makes little sense to test on larger thread cycles as our pass rates exponentially approach zero, yet this effect demonstrates how destructive non-determinstic threads become as we handle more and more data.

## Test Results on 10-thread cycle (9/25)
On 1,000,000 tests of a 10-thread cycle on a 1-thread operation, we obtain a pass rate of 100.000%. <br>
On 1,000,000 tests of a 10-thread cycle on a 2-thread operation, we obtain a pass rate of 66.4875%. <br>
On 1,000,000 tests of a 10-thread cycle on a 3-thread operation, we obtain a pass rate of 64.5759%. <br>
On 1,000,000 tests of a 10-thread cycle on a 4-thread operation, we obtain a pass rate of 61.3893%. <br>
On 1,000,000 tests of a 10-thread cycle on a 5-thread operation, we obtain a pass rate of 55.8582%. <br>
On 1,000,000 tests of a 10-thread cycle on a 8-thread operation, we obtain a pass rate of 29.9274%. <br>
On 1,000,000 tests of a 10-thread cycle on a 9-thread operation, we obtain a pass rate of 27.2472%. <br>
On 1,000,000 tests of a 10-thread cycle on a 10-thread operation, we obtain a pass rate of 24.2407%. <br>

Conclusion: As we increase the number of threads that we join in our tests, our pass rate decreases exponentially.