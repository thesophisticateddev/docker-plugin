/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dockerclient.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <KLocalizedString>

Q_DECLARE_LOGGING_CATEGORY(DOCKER_PLUGIN)
Q_LOGGING_CATEGORY(DOCKER_PLUGIN_CLIENT, "docker-plugin.client")

DockerClient::DockerClient(QObject *parent)
    : QObject(parent)
{
}

DockerClient::~DockerClient()
{
}

void DockerClient::checkAvailability()
{
    qCDebug(DOCKER_PLUGIN_CLIENT) << "Checking Docker availability...";
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    proc->setArguments({QStringLiteral("info"),
                        QStringLiteral("--format"),
                        QStringLiteral("{{json .}}")});

    connect(proc, &QProcess::finished, this,
            [this, proc](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                if (exitCode == 0) {
                    qCDebug(DOCKER_PLUGIN_CLIENT) << "Docker is available";
                    Q_EMIT availabilityChanged(DockerAvailability::Available, QString());
                } else {
                    const DockerAvailability av = classifyError(stdErr);
                    qCWarning(DOCKER_PLUGIN_CLIENT) << "Docker check failed, exit code:" << exitCode
                                                     << "stderr:" << stdErr.trimmed();
                    Q_EMIT availabilityChanged(av, stdErr.trimmed());
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError error) {
                proc->deleteLater();
                if (error == QProcess::FailedToStart) {
                    Q_EMIT availabilityChanged(DockerAvailability::NotFound,
                                               i18n("'docker' executable not found in PATH"));
                } else {
                    Q_EMIT availabilityChanged(DockerAvailability::Error,
                                               i18n("Failed to start docker process"));
                }
            });

    proc->start();
}

void DockerClient::listContainers(bool showAll)
{
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    QStringList args = {QStringLiteral("ps"),
                        QStringLiteral("--format"),
                        QStringLiteral("{{json .}}")};
    if (showAll) {
        args.append(QStringLiteral("-a"));
    }
    proc->setArguments(args);

    connect(proc, &QProcess::finished, this,
            [this, proc](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QByteArray stdOut = proc->readAllStandardOutput();
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                if (exitCode != 0) {
                    Q_EMIT availabilityChanged(DockerAvailability::Error, stdErr.trimmed());
                    return;
                }

                const QList<ContainerInfo> containers = parseContainerOutput(stdOut);
                Q_EMIT containersUpdated(containers);
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError /*error*/) {
                proc->deleteLater();
                Q_EMIT availabilityChanged(DockerAvailability::Error,
                                           i18n("Failed to run 'docker ps'"));
            });

    proc->start();
}

void DockerClient::startContainer(const QString &containerId)
{
    runContainerCommand(QStringLiteral("start"), containerId,
                        {QStringLiteral("start"), containerId});
}

void DockerClient::stopContainer(const QString &containerId)
{
    runContainerCommand(QStringLiteral("stop"), containerId,
                        {QStringLiteral("stop"), containerId});
}

void DockerClient::removeContainer(const QString &containerId)
{
    runContainerCommand(QStringLiteral("remove"), containerId,
                        {QStringLiteral("rm"), containerId});
}

void DockerClient::inspectContainer(const QString &containerId)
{
    qCDebug(DOCKER_PLUGIN_CLIENT) << "Inspecting container:" << containerId;
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    proc->setArguments({QStringLiteral("inspect"), containerId});

    connect(proc, &QProcess::finished, this,
            [this, proc, containerId](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QString stdOut = QString::fromUtf8(proc->readAllStandardOutput());
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                if (exitCode == 0) {
                    Q_EMIT inspectResult(containerId, stdOut);
                } else {
                    Q_EMIT commandFinished(QStringLiteral("inspect"), containerId, false, stdErr.trimmed());
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc, containerId](QProcess::ProcessError /*error*/) {
                proc->deleteLater();
                Q_EMIT commandFinished(QStringLiteral("inspect"), containerId, false,
                                       i18n("Failed to run 'docker inspect'"));
            });

    proc->start();
}

void DockerClient::runContainerCommand(const QString &action, const QString &containerId,
                                        const QStringList &args)
{
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    proc->setArguments(args);

    connect(proc, &QProcess::finished, this,
            [this, proc, action, containerId](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QString stdOut = QString::fromUtf8(proc->readAllStandardOutput());
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                Q_EMIT commandFinished(action, containerId, exitCode == 0,
                                       exitCode == 0 ? stdOut.trimmed() : stdErr.trimmed());
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc, action, containerId](QProcess::ProcessError /*error*/) {
                proc->deleteLater();
                Q_EMIT commandFinished(action, containerId, false,
                                       i18n("Failed to run 'docker %1'", action));
            });

    proc->start();
}

DockerAvailability DockerClient::classifyError(const QString &stdErr) const
{
    if (stdErr.contains(QLatin1String("permission denied"), Qt::CaseInsensitive)) {
        return DockerAvailability::PermissionDenied;
    }
    if (stdErr.contains(QLatin1String("Cannot connect"), Qt::CaseInsensitive)
        || stdErr.contains(QLatin1String("Is the docker daemon running"), Qt::CaseInsensitive)) {
        return DockerAvailability::DaemonDown;
    }
    return DockerAvailability::Error;
}

QList<ContainerInfo> DockerClient::parseContainerOutput(const QByteArray &data) const
{
    QList<ContainerInfo> result;
    const QList<QByteArray> lines = data.split('\n');

    for (const QByteArray &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qCWarning(DOCKER_PLUGIN_CLIENT) << "Failed to parse container JSON line:"
                                             << parseError.errorString();
            continue;
        }

        const QJsonObject obj = doc.object();
        ContainerInfo info;
        info.id = obj.value(QLatin1String("ID")).toString();
        info.name = obj.value(QLatin1String("Names")).toString();
        info.image = obj.value(QLatin1String("Image")).toString();
        info.status = obj.value(QLatin1String("Status")).toString();
        info.ports = obj.value(QLatin1String("Ports")).toString();
        result.append(info);
    }

    return result;
}

#include "moc_dockerclient.cpp"
