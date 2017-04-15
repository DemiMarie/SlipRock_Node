{ 'targets':
  [
      {
          'include_dirs': [
              '<!(node -e \'require("nan")\')',
              'SlipRock/src',
          ],
          'target_name': 'sliprock',
          'sources': [
              'src/sliprock.cc',
          ],
          'conditions': [
              ['OS=="linux"', {
                  'cflags': [
                      '-Wall',
                      '-Wextra',
                      '-pedantic',
                      '-Werror',
                  ],
              }],
              ['OS=="win"', {
                  'sources': [
                      'SlipRock/src/sliprock_windows.c',
                  ],
              }],
              ['OS!="win"', {
                  'sources': [
                      'SlipRock/src/sliprock.c',
                  ],
                  'libraries': [
                      '-lsodium',
                  ],
              }],
          ]
      }
  ],
}
