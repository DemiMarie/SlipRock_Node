{ 'targets':
  [
      {
          'include_dirs': [
              '<!(node -e \'require("nan")\')',
              'SlipRock/src',
          ],
          'target_name': 'bindings',
          'sources': [
              'src/sliprock.cc',
          ],
          'link_dirs': [
              'SlipRock/build/src',
          ],
          'link_settings': {
              'libraries': [
                  '-L<!(pwd)/SlipRock/build/src/',
                  '-lsliprock',
                  '-lsodium',
              ],
          },
          'condition': [
              'OS=="linux"', {
                  'cflags': [
                      '-std=c++14'
                  ]
              }
          ]
      }
  ],
}
