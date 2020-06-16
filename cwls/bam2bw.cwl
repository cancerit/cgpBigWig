#!/usr/bin/env cwl-runner
cwlVersion: v1.0

class: CommandLineTool

id: "cgpbigwig-bam2bw"
label: "cgpBigWig bam2bw flow"

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bam2bw tool.

  bam2bw documentation can be found [here](https://github.com/cancerit/cgpBigWig#bam2bw)

  In order to run the example found in `example/bam2bw.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)
  and change the reference path in bam2bw.json

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

baseCommand: bam2bw

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"
  - class: InlineJavascriptRequirement

inputs:
  input_path:
    type: File
    doc: "Path to the input [b|cr]am file"
    secondaryFiles: $(self.basename + self.nameext.replace('m','i'))
    inputBinding:
      prefix: --input
      position: 1
  filter:
    type: int?
    doc: "SAM flags to filter"
    inputBinding:
      prefix: --filter
  region:
    type: File?
    doc: "A bed file of regions over which to produce the bigwig file"
    inputBinding:
      prefix: --region
  include_zeros:
    type: boolean?
    doc: "Include zero coverage regions as additional entries to the bw file"
    inputBinding:
      prefix: --include-zeroes
  reference:
    type: File?
    doc: "Core reference genome.fa file (required for cram if ref_path cannot be resolved)"
    secondaryFiles:
    - .fai
    inputBinding:
      prefix: --reference
  overlap:
    type: boolean
    doc: "Flag to check for overlapping reads"
    inputBinding:
      prefix: --overlap

outputs:
  output:
    type: File
    outputBinding:
      glob: output.bam.bw

$schemas:
  - https://schema.org/version/latest/schema.rdf

$namespaces:
  s: http://schema.org/


s:codeRepository: https://github.com/cancerit/dockstore-cgpbigwig
s:license: https://spdx.org/licenses/AGPL-3.0-only

s:author:
  - class: s:Person
    s:identifier: https://orcid.org/0000-0002-0407-0386
    s:email: mailto:drj@sanger.ac.uk
    s:name: David Jones

dct:creator:
  "@id": "https://orcid.org/0000-0002-0407-0386"
  foaf:name: David Jones
  foaf:mbox: "drj@sanger.ac.uk"
