param(
    [int]$pr = 0
)

if ($pr -eq 0) {
    $pr = gh pr view --json number --jq '.number' 2>$null
    if (-not $pr) {
        Write-Error "No PR number provided and no open PR found for the current branch."
        exit 1
    }
    $pr = [int]$pr
}

$query = @"
  query(`$owner:String!, `$repo:String!, `$pr:Int!) {
    repository(owner:`$owner, name:`$repo) {
      pullRequest(number:`$pr) {
        reviewThreads(first: 100) {
          nodes {
            isResolved
            comments(first: 100) {
              nodes {
                author { login }
                body
                path
                line
              }
            }
          }
        }
      }
    }
  }
"@

gh api graphql -f owner="shyney7" -f repo="ZephyrSense" -F pr=$pr -f query=$query `
  --jq '.data.repository.pullRequest.reviewThreads.nodes[] | select(.isResolved == false) | "=== \(.comments.nodes[0].path) (Line \(.comments.nodes[0].line)) ===\n" + (.comments.nodes | map("  [\(.author.login)]: \(.body)") | join("\n")) + "\n---"'
