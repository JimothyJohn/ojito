#!/usr/bin/env bash

curl http://10.0.0.31/predictions -X POST \
    -H "Content-Type: application/json" \
    -d '{"input": {"image" : "https://pbxt.replicate.delivery/IWHv3cYJ7CAVPFJN5M9JohLfLr2XaGxXgh5ykca1kvourUZV/taylor1.jpg"}}' | \
    jq .output