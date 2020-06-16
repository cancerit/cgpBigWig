#!/usr/bin/env cwl-runner

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bam2bedgraph tool.

  bam2bedgraph documentation can be found [here](https://github.com/cancerit/cgpBigWig#bam2bedgraph)

  In order to run the example found in `example/bam2bedgraph.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

cwlVersion: v1.0

class: CommandLineTool

id: "cgpbigwig-bam2bedgraph"
label: "cgpbigwig-bam2bedgraph"

baseCommand: bam2bedgraph

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"

inputs:
  input_path:
    type: File
    doc: "Path to bam/cram input file"
    inputBinding:
      prefix: --input
      position: 1
  region:
    type: string?
    doc: "Region in bam to access"
    inputBinding:
      prefix: --region
  filter:
    type: int
    doc: "Ignore reads with the filter flags"
    inputBinding:
      prefix: --filter
  overlap:
    type: boolean
    doc: "Flag to check for overlapping reads"
    inputBinding:
      prefix: --overlap

outputs:
  output:
    type: stdout

$namespaces:
  s: http://schema.org/

$schemas:
  - https://schema.org/version/latest/schema.rdf

s:codeRepository: https://github.com/cancerit/dockstore-cgpbigwig
s:license: https://spdx.org/licenses/GPL-3.0

s:author:
  - class: s:Person
    s:identifier: https://orcid.org/0000-0002-0407-0386
    s:email: mailto:drj@sanger.ac.uk
    s:name: David Jones

dct:creator:
"@id": "https://orcid.org/0000-0002-0407-0386"
foaf:name: David Jones
foaf:mbox: "drj@sanger.ac.uk"
