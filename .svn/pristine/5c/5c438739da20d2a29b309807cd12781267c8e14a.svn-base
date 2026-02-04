#!/bin/sh

if test "a$1" = "a-d"
then
  cat 'test.input' | ./radre -d 'test.config'
else
  echo '****************************************'
  echo "**  It's demo mode.                   **"
  echo "**  Type:                             **"
  echo "**  $0 -d"
  echo '**  to run these tests in debug mode. **'
  echo '****************************************'
  cat 'test.input' | ./radre 'test.config'
fi

