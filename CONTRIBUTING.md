# Contributing

Contributions are welcome, and they are greatly appreciated! Every
little bit helps, and credit will always be given.

This project could always use more documentation, whether as part of the
README, in docstrings, or even on the web in blog posts articles, and such.

Please learn about [`semantic versioning`][semver].

# Contents

- [Contributing](#contributing)
- [Contents](#contents)
- [Development](#development)
    - [Bug reports, Feature requests and feedback](#bug-reports-feature-requests-and-feedback)
    - [Pull Request Guidelines](#pull-request-guidelines)
- [Creating a New Release](#creating-a-new-release)

# Development

Set up for local development:

1. Clone your cookiecutter-cli locally:

    ```
    git clone https://github.com/cancerit/cgpBigWig.git
    ```

2. Checkout to development branch:

    ```
    git checkout develop
    git pull
    ```

3. Create a branch for local development:

    ```
    git checkout -b name-of-your-bugfix-or-feature
    ```

    Now you can make your changes locally.

4. If you contributed a new feature, create a test in:

    ```
    cgpBigWig/c/c_tests/
    ```

5. Commit your changes and push your branch to GitHub:

    ```
    git add .
    git commit -m "Your meaningful description"
    git push origin name-of-your-bugfix-or-feature
    ```

6. Submit a pull request through the GitHub website.

## Bug reports, Feature requests and feedback

Go ahead and file an issue at https://github.com/cancerit/cgpBigWig/issues.

If you are proposing a **feature**:

* Explain in detail how it would work.
* Keep the scope as narrow as possible, to make it easier to implement.
* Remember that this is a volunteer-driven project, and that code contributions are welcome :)

When reporting a **bug** please include:

* Your operating system name and version.
* Any details about your local setup that might be helpful in troubleshooting.
* Detailed steps to reproduce the bug.

## Pull Request Guidelines

If you need some code review or feedback while you're developing the code just make the pull request.

For merging, you should:

1. Include passing tests.
2. Update documentation when there's new API, functionality etc.

# Creating a New Release

Please follow this steps before publishing a new release:

1. Update [INSTALL](INSTALL.md) and [Dockerfile](Dockerfile) if new dependencies are required.

2. Publish docker image:

        docker build -t cancerit/cgp-bigwig:`cat VERSION.txt` .
        docker push cancerit/cgp-bigwig:`cat VERSION.txt`

<!-- References -->

[semver]: http://semver.org/
